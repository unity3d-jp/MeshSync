#include "pch.h"
#include "msISceneCache.h"
#include "msMisc.h"
#include "Utils/msDebug.h"

#ifdef msEnableSceneCache
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

    m_records.reserve(512);
    for (;;) {
        // enumerate all scene headers
        CacheFileSceneHeader sh;
        m_ist->read((char*)&sh, sizeof(sh));
        if (sh.buffer_count == 0) {
            // empty header is a terminator
            break;
        }
        else {
            SceneRecord rec;
            rec.time = sh.time;

            rec.buffer_sizes.resize_discard(sh.buffer_count);
            m_ist->read((char*)rec.buffer_sizes.data(), rec.buffer_sizes.size_in_byte());
            rec.pos = (uint64_t)m_ist->tellg();

            rec.buffer_size_total = 0;
            for (auto s : rec.buffer_sizes)
                rec.buffer_size_total += s;

            rec.segments.resize(sh.buffer_count);

            m_records.emplace_back(std::move(rec));
            m_ist->seekg(rec.buffer_size_total, std::ios::cur);
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
ScenePtr ISceneCacheImpl::getByIndexImpl(size_t scene_index, bool wait_preload)
{
    if (!valid() || scene_index >= m_records.size())
        return nullptr;

    auto& rec = m_records[scene_index];
    if (wait_preload && rec.preload.valid()) {
        // wait preload
        rec.preload.wait();
        rec.preload = {};
    }

    auto& ret = rec.scene;
    if (ret)
        return ret; // already loaded

    auto time_begin = mu::Now();

    size_t seg_count = rec.buffer_sizes.size();
    rec.segments.resize(seg_count);

    {
        // get exclusive file access
        std::unique_lock<std::mutex> lock(m_mutex);

        m_ist->seekg(rec.pos, std::ios::beg);
        for (size_t si = 0; si < seg_count; ++si) {
            auto& seg = rec.segments[si];
            {
                msDbgTimer("ISceneCacheImpl: [%d] read segment (%d)", (int)scene_index, (int)si);
                seg.encoded_buf.resize(rec.buffer_sizes[si]);
                m_ist->read(seg.encoded_buf.data(), seg.encoded_buf.size());
            }

            seg.task = std::async(std::launch::async, [this, &seg, scene_index, si]() {
                msDbgTimer("ISceneCacheImpl: [%d] decode segment (%d)", (int)scene_index, (int)si);

                RawVector<char> tmp_buf;
                m_encoder->decode(tmp_buf, seg.encoded_buf);

                auto ret = Scene::create();
                MemoryStream scene_buf(std::move(tmp_buf));
                try {
                    ret->deserialize(scene_buf);

                    // keep scene buffer alive. Meshes will use it as vertex buffers
                    ret->scene_buffers.push_back(scene_buf.moveBuffer());
                    seg.segment = ret;
                }
                catch (std::runtime_error& e) {
                    msLogError("exception: %s\n", e.what());
                    ret = nullptr;
                    seg.error = true;
                }
            });
        }
    }

    // concat segmented scenes
    for (size_t si = 0; si < seg_count; ++si) {
        auto& seg = rec.segments[si];
        seg.task.wait();
        if (seg.error)
            break;

        if (si == 0)
            ret = seg.segment;
        else
            ret->concat(*seg.segment, true);
    }
    if (ret)
        std::sort(ret->entities.begin(), ret->entities.end(), [](auto& a, auto& b) { return a->id < b->id; });
    rec.segments.clear();

    {
        msDbgTimer("ISceneCacheImpl: [%d] merge", (int)scene_index);
        if (m_header.oscs.strip_unchanged && m_base_scene && ret) {
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
    {
        // do import
        msDbgTimer("ISceneCacheImpl: [%d] import", (int)scene_index);
        ret->import(m_iscs.sis);
    }
    rec.load_time_ms = mu::NS2MS(mu::Now() - time_begin);

    // push & pop history
    if (!m_header.oscs.strip_unchanged || scene_index != 0) {
        m_history.push_back(scene_index);
        if (m_history.size() > m_iscs.max_history) {
            m_records[m_history.front()].scene.reset();
            m_history.pop_front();
        }
    }
    return ret;
}

ScenePtr ISceneCacheImpl::postprocess(ScenePtr& sp, size_t scene_index)
{
    if (!sp)
        return sp;

    ScenePtr ret;

    // m_last_scene and m_last_diff keep reference counts and keep scenes alive.
    // (plugin APIs return raw scene pointers. someone needs to keep its reference counts)
    if (m_last_scene && m_iscs.enable_diff) {
        msDbgTimer("ISceneCacheImpl: [%d] diff", (int)scene_index);
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
    if (!valid() || time == m_last_time)
        return nullptr;

    ScenePtr ret;

    const int scene_count = (int)m_records.size();

    auto time_range = getTimeRange();
    if (time <= time_range.start) {
        int si = 0;
        if ((!interpolation && m_last_index == si) ||
            (m_last_index == si && m_last_index2 == si))
            return nullptr;

        m_last_index = m_last_index2 = si;
        ret = getByIndexImpl(si);
    }
    else if (time >= time_range.end) {
        int si =  scene_count - 1;
        if ((!interpolation && m_last_index == si) ||
            (m_last_index == si && m_last_index2 == si))
            return nullptr;

        m_last_index = m_last_index2 = si;
        ret = getByIndexImpl(si);
    }
    else {
        int si = timeToIndex(time);
        if (interpolation) {
            auto t1 = m_records[si + 0].time;
            auto t2 = m_records[si + 1].time;

            kickPreload(si + 1);
            auto s1 = getByIndexImpl(si + 0);
            auto s2 = getByIndexImpl(si + 1);

            {
                msDbgTimer("ISceneCacheImpl: [%d] lerp", (int)si);
                float t = (time - t1) / (t2 - t1);
                ret = Scene::create();
                ret->lerp(*s1, *s2, t);
                // keep a reference for s1 (s2 is not needed)
                ret->data_sources.push_back(s1);
            }

            m_last_index = si;
            m_last_index2 = si + 1;
        }
        else {
            if (si == m_last_index)
                return nullptr;
            ret = getByIndexImpl(si);

            m_last_index = m_last_index2 = si;
        }
    }
    m_last_time = time;
    return postprocess(ret, m_last_index2);
}

void ISceneCacheImpl::refresh()
{
    m_last_index = m_last_index2  = -1;
    m_last_time = -1.0f;
    m_last_scene = nullptr;
    m_last_diff = nullptr;
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
#endif // msEnableSceneCache
