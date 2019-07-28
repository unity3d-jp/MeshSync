#include "pch.h"
#include "msISceneCache.h"
#include "msMisc.h"
#include "Utils/msDebug.h"

namespace ms {

ISceneCacheImpl::ISceneCacheImpl(StreamPtr ist, const ISceneCacheSettings& iscs)
{
    m_ist = ist;
    m_iscs = iscs;
    if (!m_ist || !(*m_ist))
        return;

    m_header.version = 0;
    m_ist->read((char*)&m_header, sizeof(m_header));
    if (m_header.version != msProtocolVersion)
        return;

    m_encoder = CreateEncoder(m_header.oscs.encoding, m_header.oscs.encoder_settings);
    if (!m_encoder) {
        // encoder associated with m_settings.encoding is not available
        return;
    }

    for (;;) {
        // enumerate all scene headers
        CacheFileSceneHeader sh;
        m_ist->read((char*)&sh, sizeof(sh));
        if (sh.size == 0) {
            // empty header is a terminator
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

    if (m_header.oscs.strip_unchanged)
        m_base_scene = getByIndexImpl(0);
}

ISceneCacheImpl::~ISceneCacheImpl()
{
}

bool ISceneCacheImpl::valid() const
{
    return !m_records.empty();
}

int ISceneCacheImpl::timeToIndex(float time) const
{
    if (!valid())
        return 0;
    auto p = std::lower_bound(m_records.begin(), m_records.end(), time, [](auto& a, float t) { return a.time < t; });
    return int(std::distance(m_records.begin(), p) - 1);
}


size_t ISceneCacheImpl::getNumScenes() const
{
    if (!valid())
        return 0;
    return m_records.size();
}

TimeRange ISceneCacheImpl::getTimeRange() const
{
    if (!valid())
        return {0.0f, 0.0f};
    return { m_records.front().time, m_records.back().time };
}

ScenePtr ISceneCacheImpl::getByIndexImpl(size_t i)
{
    if (!valid() || i >= m_records.size())
        return nullptr;

    auto& rec = m_records[i];
    auto& ret = rec.scene;
    if (ret)
        return ret;


    // read buffer
    {
        msDbgTimer("ISceneCacheImpl: read");

        m_encoded_buf.resize(rec.size);
        m_ist->seekg(rec.pos, std::ios::beg);
        m_ist->read(m_encoded_buf.data(), m_encoded_buf.size());
    }

    // decode buffer
    {
        msDbgTimer("ISceneCacheImpl: decode");

        m_encoder->decode(m_tmp_buf, m_encoded_buf);
        m_scene_buf.swap(m_tmp_buf);
    }

    // deserialize scene
    try {
        msDbgTimer("ISceneCacheImpl: deserialize");

        ret = Scene::create();
        ret->deserialize(m_scene_buf);
        if (m_header.oscs.strip_unchanged && m_base_scene)
            ret->merge(*m_base_scene);
    }
    catch (std::runtime_error& e) {
        msLogError("exception: %s\n", e.what());
        ret = nullptr;
    }

    if (!m_header.oscs.strip_unchanged || i != 0) {
        m_history.push_back(i);
        if (m_history.size() >= m_iscs.max_history) {
            m_records[m_history.front()].scene.reset();
            m_history.pop_front();
        }
    }
    return ret;
}

ScenePtr ISceneCacheImpl::postprocess(ScenePtr& sp)
{
    msDbgTimer("ISceneCacheImpl: postprocess");

    if (m_iscs.convert_scenes) {
        sp->import(m_iscs.sis);
    }

    // m_last_scene and m_last_diff keep reference counts and keep scenes alive.
    // (plugin APIs return raw scene pointers. someone needs to keep its reference counts)
    if (m_last_scene && m_iscs.enable_diff) {
        m_last_diff = Scene::create();
        m_last_diff->diff(*sp, *m_last_scene);
        m_last_scene = sp;
        return m_last_diff;
    }
    else {
        m_last_diff = nullptr;
        m_last_scene = sp;
        return sp;
    }
}

ScenePtr ISceneCacheImpl::getByIndex(size_t i)
{
    if (!valid())
        return nullptr;

    auto ret = getByIndexImpl(i);
    return postprocess(ret);
}

ScenePtr ISceneCacheImpl::getByTime(float time, bool interpolation)
{
    if (!valid())
        return nullptr;

    ScenePtr ret;

    auto time_range = getTimeRange();
    if (time <= time_range.start) {
        ret = getByIndexImpl(0);
    }
    else if (time >= time_range.end) {
        ret = getByIndexImpl(m_records.size() - 1);
    }
    else {
        int i = timeToIndex(time);
        if (interpolation) {
            auto t1 = m_records[i + 0].time;
            auto t2 = m_records[i + 1].time;
            auto s1 = getByIndexImpl(i + 0);
            auto s2 = getByIndexImpl(i + 1);

            float t = (time - t1) / (t2 - t1);
            ret = Scene::create();
            ret->lerp(*s1, *s2, t);
        }
        else {
            ret = getByIndexImpl(i);
        }
    }

    return postprocess(ret);
}


ISceneCacheFile::ISceneCacheFile(const char *path, const ISceneCacheSettings& iscs)
    : super(createStream(path, iscs), iscs)
{
}

ISceneCacheFile::StreamPtr ISceneCacheFile::createStream(const char *path, const ISceneCacheSettings& iscs)
{
    if (!path)
        return nullptr;

    if (iscs.preload_entire_file) {
        RawVector<char> buf;
        if (FileToByteArray(path, buf)) {
            auto ret = std::make_shared<MemoryStream>();
            ret->swap(buf);
            return ret;
        }
    }
    else {
        auto ret = std::make_shared<std::ifstream>();
        ret->open(path, std::ios::binary);
        return *ret ? ret : nullptr;
    }
    return nullptr;
}


ISceneCache* OpenISceneCacheFileRaw(const char *path, const ISceneCacheSettings& iscs)
{
    auto ret = new ISceneCacheFile(path, iscs);
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
