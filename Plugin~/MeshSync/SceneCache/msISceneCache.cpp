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

    size_t scene_count = m_records.size();
    std::sort(m_records.begin(), m_records.end(), [](auto& a, auto& b) { return a.time < b.time; });

    m_time_curve = AnimationCurve::create();
    TAnimationCurve<float> curve(m_time_curve);
    curve.resize(scene_count);
    for (size_t i = 0; i < scene_count; ++i) {
        auto& kvp = curve[i];
        kvp.time = kvp.value = m_records[i].time;
    }

    {
        RawVector<char> encoded_buf, tmp_buf;

        // read meta data
        CacheFileMetaHeader mh;
        m_ist->read((char*)&mh, sizeof(mh));

        encoded_buf.resize(mh.size);
        m_ist->read(encoded_buf.data(), encoded_buf.size());

        m_encoder->decode(tmp_buf, encoded_buf);
        m_entity_meta.resize_discard(tmp_buf.size() / sizeof(CacheFileEntityMeta));
        tmp_buf.copy_to((char*)m_entity_meta.data());
    }

    if (m_header.oscs.strip_unchanged)
        m_base_scene = getByIndexImpl(0);

    //preloadAll(); // for test
}

ISceneCacheImpl::~ISceneCacheImpl()
{
    waitAllPreloads();
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

void ISceneCacheImpl::preloadAll()
{
    size_t n = m_records.size();
    m_iscs.max_history = (int)n;
    for (size_t i = 0; i < n; ++i)
        kickPreload(i);
}


float ISceneCacheImpl::getSampleRate() const
{
    return m_header.oscs.sample_rate;
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

float ISceneCacheImpl::getTime(size_t i) const
{
    if (!valid())
        return 0.0f;
    return m_records[i].time;
}

// thread safe
ScenePtr ISceneCacheImpl::getByIndexImpl(size_t i, bool wait_preload)
{
    if (!valid() || i >= m_records.size())
        return nullptr;

    auto& rec = m_records[i];
    if (wait_preload && rec.preload.valid()) {
        // wait preload
        rec.preload.wait();
        rec.preload = {};
    }

    auto& ret = rec.scene;
    if (ret)
        return ret; // already loaded

    auto time_begin = mu::Now();
    RawVector<char> encoded_buf, tmp_buf;

    // read buffer
    {
        msDbgTimer("ISceneCacheImpl: read");

        encoded_buf.resize(rec.size);
        m_ist->seekg(rec.pos, std::ios::beg);
        m_ist->read(encoded_buf.data(), encoded_buf.size());
    }

    // decode buffer
    {
        msDbgTimer("ISceneCacheImpl: decode");

        m_encoder->decode(tmp_buf, encoded_buf);
    }

    // deserialize scene
    try {
        msDbgTimer("ISceneCacheImpl: deserialize");

        ret = Scene::create();
        MemoryStream scene_buf(std::move(tmp_buf));
        ret->deserialize(scene_buf);

        if (m_header.oscs.strip_unchanged && m_base_scene) {
            // set cache flags
            size_t n = ret->entities.size();
            if (m_entity_meta.size() == n) {
                enumerate(m_entity_meta, ret->entities, [](auto& meta, auto& e) {
                    if (meta.id == e->id) {
                        e->cache_flags.constant = meta.constant;
                        e->cache_flags.constant_topology = meta.constant_topology;
                    }
                });
            }

            // merge
            ret->merge(*m_base_scene);
        }
    }
    catch (std::runtime_error& e) {
        msLogError("exception: %s\n", e.what());
        ret = nullptr;
    }
    rec.load_time_ms = mu::NS2MS(mu::Now() - time_begin);

    // push & pop history
    if (!m_header.oscs.strip_unchanged || i != 0) {
        m_history.push_back(i);
        if (m_history.size() >= m_iscs.max_history) {
            m_records[m_history.front()].scene.reset();
            m_history.pop_front();
        }
    }
    return ret;
}

ScenePtr ISceneCacheImpl::postprocess(ScenePtr& sp, size_t scene_index)
{
    msDbgTimer("ISceneCacheImpl: postprocess");

    // do import
    if (m_iscs.convert_scenes) {
        sp->import(m_iscs.sis);
    }

    ScenePtr ret;

    // m_last_scene and m_last_diff keep reference counts and keep scenes alive.
    // (plugin APIs return raw scene pointers. someone needs to keep its reference counts)
    if (m_last_scene && m_iscs.enable_diff) {
        m_last_diff = Scene::create();
        m_last_diff->diff(*sp, *m_last_scene);
        m_last_scene = sp;
        ret = m_last_diff;
    }
    else {
        m_last_diff = nullptr;
        m_last_scene = sp;
        ret = sp;
    }

    // preload
    if (m_iscs.preload_scenes && scene_index + 1 < m_records.size())
        kickPreload(scene_index + 1);

    return ret;
}

bool ISceneCacheImpl::kickPreload(size_t i)
{
    auto& rec = m_records[i];
    if (rec.scene || rec.preload.valid())
        return false; // already loaded or loading

    rec.preload = std::async(std::launch::async, [this, i]() { getByIndexImpl(i, false); });
    return true;
}

void ISceneCacheImpl::waitAllPreloads()
{
    for (auto& rec : m_records) {
        if (rec.preload.valid()) {
            rec.preload.wait();
            rec.preload = {};
        }
    }
}

ScenePtr ISceneCacheImpl::getByIndex(size_t i)
{
    if (!valid())
        return nullptr;

    auto ret = getByIndexImpl(i);
    return postprocess(ret, i);
}

ScenePtr ISceneCacheImpl::getByTime(float time, bool interpolation)
{
    if (!valid())
        return nullptr;

    ScenePtr ret;

    const int scene_count = (int)m_records.size();
    int scene_index = 0;

    auto time_range = getTimeRange();
    if (time <= time_range.start) {
        scene_index = 0;
        ret = getByIndexImpl(scene_index);
    }
    else if (time >= time_range.end) {
        scene_index = scene_count - 1;
        ret = getByIndexImpl(scene_index);
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
            scene_index = i + 1;
        }
        else {
            ret = getByIndexImpl(i);
            scene_index = i;
        }
    }

    return postprocess(ret, scene_index);
}

const AnimationCurvePtr ISceneCacheImpl::getTimeCurve() const
{
    return m_time_curve;
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
