#include "pch.h"
#include "msSceneCache.h"
#include "msSceneCacheImpl.h"
#include "msMisc.h"

namespace ms {

static BufferEncoderPtr CreateEncoder(SceneCacheEncoding encoding)
{
    BufferEncoderPtr ret;
    switch (encoding) {
    case SceneCacheEncoding::ZSTD: ret = CreateZSTDEncoder(); break;
    default: break;
    }
    return ret;
}

OSceneCache::~OSceneCache()
{
}

ISceneCache::~ISceneCache()
{
}

OSceneCacheImpl::OSceneCacheImpl()
{
}

OSceneCacheImpl::~OSceneCacheImpl()
{
    if (valid()) {
        flush();

        // add terminator
        auto terminator = CacheFileSceneHeader::terminator();
        m_ost->write((char*)&terminator, sizeof(terminator));
    }
}

void OSceneCacheImpl::addScene(ScenePtr scene, float time)
{
    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_queue.push_back({ scene, time });
    }
    doWrite();
}

void OSceneCacheImpl::flush()
{
    doWrite();
    if (m_task.valid())
        m_task.wait();
}

bool OSceneCacheImpl::isWriting()
{
    return m_task.valid() && m_task.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

bool OSceneCacheImpl::prepare(ostream_ptr ost, const OSceneCacheSettings& settings)
{
    m_ost = ost;
    m_settings = settings;
    if (!m_ost)
        return false;

    m_encoder = CreateEncoder(m_settings.encoding);
    if (!m_encoder) {
        m_settings.encoding = SceneCacheEncoding::Plain;
        m_encoder = CreatePlainEncoder();
    }

    CacheFileHeader header;
    header.settings = m_settings;
    m_ost->write((char*)&header, sizeof(header));
    return valid();
}

bool OSceneCacheImpl::valid() const
{
    return this && m_ost;
}

void OSceneCacheImpl::doWrite()
{
    auto body = [this]() {
        for (;;) {
            SceneRecord desc;
            {
                std::unique_lock<std::mutex> l(m_mutex);
                if (m_queue.empty()) {
                    break;
                }
                else {
                    desc = m_queue.back();
                    m_queue.pop_back();
                }
            }
            if (!desc.scene)
                continue;

            if (m_settings.strip_unchanged) {
                if (!m_base_scene)
                    m_base_scene = desc.scene;
                else
                    desc.scene->strip(*m_base_scene);
            }

            // serialize
            m_scene_buf.reset();
            desc.scene->serialize(m_scene_buf);
            m_scene_buf.flush();

            // encode
            m_encoder->encode(m_encoded_buf, m_scene_buf.getBuffer());

            // write
            CacheFileSceneHeader header{ m_encoded_buf.size(), desc.time };
            m_ost->write((char*)&header, sizeof(header));
            m_ost->write(m_encoded_buf.data(), m_encoded_buf.size());
        
        }
    };

    {
        std::unique_lock<std::mutex> l(m_mutex);
        if (!m_queue.empty() && !isWriting()) {
            m_task = std::async(std::launch::async, body);
        }
    }
}


ISceneCacheImpl::ISceneCacheImpl()
{
}

ISceneCacheImpl::~ISceneCacheImpl()
{
}

void ISceneCacheImpl::setImportSettings(const SceneImportSettings& cv)
{
    m_import_settings = cv;
}

const SceneImportSettings & ISceneCacheImpl::getImportSettings() const
{
    return m_import_settings;
}

bool ISceneCacheImpl::prepare(istream_ptr ist)
{
    m_ist = ist;
    if (!m_ist)
        return false;

    CacheFileHeader header;
    header.version = 0;
    m_ist->read((char*)&header, sizeof(header));
    m_osc_settings = header.settings;

    if (header.version != msProtocolVersion)
        return false;

    m_encoder = CreateEncoder(m_osc_settings.encoding);
    if (!m_encoder) {
        // encoder associated with m_settings.encoding is not available
        return false;
    }

    for (;;) {
        CacheFileSceneHeader sh;
        m_ist->read((char*)&sh, sizeof(sh));
        if (sh.size == 0) {
            break;
        }
        else {
            SceneRecord desc;
            desc.pos = (uint64_t)m_ist->tellg();
            desc.size = sh.size;
            desc.time = sh.time;
            m_records.push_back(desc);

            m_ist->seekg(sh.size, std::ios::cur);
        }
    }
    std::sort(m_records.begin(), m_records.end(), [](auto& a, auto& b) { return a.time < b.time; });

    if (m_osc_settings.strip_unchanged)
        m_base_scene = getByIndexImpl(0, false);

    return valid();
}

bool ISceneCacheImpl::valid() const
{
    return this && !m_records.empty();
}

size_t ISceneCacheImpl::getNumScenes() const
{
    if (!valid())
        return 0;
    return m_records.size();
}

std::tuple<float, float> ISceneCacheImpl::getTimeRange() const
{
    if (!valid())
        return {0.0f, 0.0f};
    return { m_records.front().time, m_records.back().time };
}

ScenePtr ISceneCacheImpl::getByIndexImpl(size_t i, bool convert)
{
    if (!valid() || i >= m_records.size())
        return nullptr;

    auto& rec = m_records[i];
    auto& ret = rec.scene;
    if (ret)
        return ret;


    // read
    m_encoded_buf.resize(rec.size);
    m_ist->seekg(rec.pos, std::ios::beg);
    m_ist->read(m_encoded_buf.data(), m_encoded_buf.size());

    // decode
    m_encoder->decode(m_tmp_buf, m_encoded_buf);
    m_scene_buf.swap(m_tmp_buf);

    // deserialize
    try {
        ret = Scene::create();
        ret->deserialize(m_scene_buf);
        if (m_osc_settings.strip_unchanged && m_base_scene)
            ret->merge(*m_base_scene);
        if (convert)
            ret->import(m_import_settings);
    }
    catch (std::runtime_error& e) {
        msLogError("exception: %s\n", e.what());
        ret = nullptr;
    }

    m_history.push_back(i);
    if (m_history.size() >= m_isc_settings.max_history) {
        m_records[m_history.front()].scene.reset();
        m_history.pop_front();
    }
    return ret;
}

ScenePtr ISceneCacheImpl::applyDiff(ScenePtr& sp)
{
    if (m_last_scene && m_isc_settings.enable_diff) {
        m_last_diff = Scene::create();
        m_last_diff->diff(*sp, *m_last_scene);
        m_last_scene = sp;
        return m_last_diff;
    }
    else {
        m_last_scene = sp;
        return sp;
    }
}

ScenePtr ISceneCacheImpl::getByIndex(size_t i)
{
    if (!valid())
        return nullptr;

    auto ret = getByIndexImpl(i, m_isc_settings.convert_scene);
    return applyDiff(ret);
}

ScenePtr ISceneCacheImpl::getByTime(float time, bool lerp)
{
    if (!valid())
        return nullptr;

    bool convert = m_isc_settings.convert_scene;

    ScenePtr ret;
    if (time <= m_records.front().time)
        ret = getByIndexImpl(0, convert);
    else if(time >= m_records.back().time)
        ret = getByIndexImpl(m_records.size() - 1, convert);
    else {
        auto i = std::distance(m_records.begin(),
            std::lower_bound(m_records.begin(), m_records.end(), time, [time](auto& a, float t) { return a.time < t; })) - 1;
        if (lerp) {
            auto t1 = m_records[i + 0].time;
            auto t2 = m_records[i + 1].time;
            auto s1 = getByIndexImpl(i + 0, convert);
            auto s2 = getByIndexImpl(i + 1, convert);

            float t = (time - t1) / (t2 - t1);
            ret = Scene::create();
            ret->lerp(*s1, *s2, t);
        }
        else {
            ret = getByIndexImpl(i, convert);
        }
    }

    return applyDiff(ret);
}


OSceneCacheFile::OSceneCacheFile(const char *path, const OSceneCacheSettings& settings)
{
    if (path) {
        auto ofs = std::make_shared<std::ofstream>();
        ofs->open(path, std::ios::binary);
        if (*ofs) {
            prepare(ofs, settings);
        }
    }
}

OSceneCache* OpenOSceneCacheFileRaw(const char *path, const OSceneCacheSettings& settings)
{
    auto ret = new OSceneCacheFile(path, settings);
    if (ret->valid()) {
        return ret;
    }
    else {
        delete ret;
        return nullptr;
    }
}
OSceneCachePtr OpenOSceneCacheFile(const char *path, const OSceneCacheSettings& settings)
{
    return OSceneCachePtr(OpenOSceneCacheFileRaw(path, settings));
}


ISceneCacheFile::ISceneCacheFile(const char *path, const ISceneCacheSettings& settings)
{
    m_isc_settings = settings;

    if (path) {
        if (m_isc_settings.preload_entire_file) {
            RawVector<char> buf;
            if (FileToByteArray(path, buf)) {
                auto ms = std::make_shared<MemoryStream>();
                ms->swap(buf);
                prepare(ms);
            }
        }
        else {
            auto ifs = std::make_shared<std::ifstream>();
            ifs->open(path, std::ios::binary);
            if (*ifs) {
                prepare(ifs);
            }
        }
    }
}

ISceneCache* OpenISceneCacheFileRaw(const char *path, const ISceneCacheSettings& settings)
{
    auto ret = new ISceneCacheFile(path, settings);
    if (ret->valid()) {
        return ret;
    }
    else {
        delete ret;
        return nullptr;
    }
}
ISceneCachePtr OpenISceneCacheFile(const char *path, const ISceneCacheSettings& settings)
{
    return ISceneCachePtr(OpenISceneCacheFileRaw(path, settings));
}

} // namespace ms
