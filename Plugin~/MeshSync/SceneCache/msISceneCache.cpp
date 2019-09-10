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

        encoded_buf.resize((size_t)mh.size);
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


int ISceneCacheImpl::getPreloadLength() const
{
    return m_iscs.preload_length;
}

void ISceneCacheImpl::setPreloadLength(int v)
{
    m_iscs.setPreloadLength(v);
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

float ISceneCacheImpl::getTime(int i) const
{
    if (!valid())
        return 0.0f;
    if (i <= 0)
        return m_records.front().time;
    if (i >= m_records.size())
        return m_records.back().time;
    return m_records[i].time;
}

int ISceneCacheImpl::getFrameByTime(float time) const
{
    if (!valid())
        return 0;
    auto p = std::lower_bound(m_records.begin(), m_records.end(), time, [](auto& a, float t) { return a.time < t; });
    if (p != m_records.end()) {
        int d = (int)std::distance(m_records.begin(), p);
        return p->time == time ? d : d - 1;
    }
    return 0;
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

    auto load_begin = mu::Now();

    size_t seg_count = rec.buffer_sizes.size();
    rec.segments.resize(seg_count);

    {
        // get exclusive file access
        std::unique_lock<std::mutex> lock(m_mutex);

        m_ist->seekg(rec.pos, std::ios::beg);
        for (size_t si = 0; si < seg_count; ++si) {
            auto& seg = rec.segments[si];
            seg.size_encoded = rec.buffer_sizes[si];

            // read segment
            {
                msProfileScope("ISceneCacheImpl: [%d] read segment (%d - %u byte)", (int)scene_index, (int)si, (uint32_t)seg.size_encoded);
                ScopedTimer timer;

                seg.encoded_buf.resize((size_t)seg.size_encoded);
                m_ist->read(seg.encoded_buf.data(), seg.encoded_buf.size());

                seg.read_time = timer.elapsed();
            }

            // launch async decode
            seg.task = std::async(std::launch::async, [this, &seg, scene_index, si]() {
                msProfileScope("ISceneCacheImpl: [%d] decode segment (%d)", (int)scene_index, (int)si);
                ScopedTimer timer;

                RawVector<char> tmp_buf;
                m_encoder->decode(tmp_buf, seg.encoded_buf);
                seg.size_decoded = tmp_buf.size();

                auto ret = Scene::create();
                MemoryStream scene_buf(std::move(tmp_buf));
                try {
                    ret->deserialize(scene_buf);

                    // keep scene buffer alive. Meshes will use it as vertex buffers
                    ret->scene_buffers.push_back(scene_buf.moveBuffer());
                    seg.segment = ret;

                    // count vertices
                    seg.vertex_count = 0;
                    for (auto& e : seg.segment->entities)
                        seg.vertex_count += e->vertexCount();
                }
                catch (std::runtime_error& e) {
                    msLogError("exception: %s\n", e.what());
                    ret = nullptr;
                    seg.error = true;
                }

                seg.decode_time = timer.elapsed();
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

    if (ret) {
        // sort entities by ID
        std::sort(ret->entities.begin(), ret->entities.end(), [](auto& a, auto& b) { return a->id < b->id; });

        // update profile data
        auto& prof = ret->profile_data;
        prof = {};
        prof.load_time = NS2MS(Now() - load_begin);
        for (auto& seg : rec.segments) {
            prof.size_encoded += seg.size_encoded;
            prof.size_decoded += seg.size_decoded;
            prof.read_time += seg.read_time;
            prof.decode_time += seg.decode_time;
            prof.vertex_count += seg.vertex_count;
        }

        {
            msProfileScope("ISceneCacheImpl: [%d] merge & import", (int)scene_index);
            ScopedTimer timer;

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

            // do import
            ret->import(m_iscs.sis);

            prof.setup_time = timer.elapsed();
        }
    }
    rec.segments.clear();

    // push & pop history
    if (!m_header.oscs.strip_unchanged || scene_index != 0) {
        m_history.push_back(scene_index);
        while (m_history.size() > m_iscs.max_history) {
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
        msProfileScope("ISceneCacheImpl: [%d] diff", (int)scene_index);
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

    // kick preload
    if (m_iscs.preload_length > 0 && scene_index + 1 < m_records.size()) {
        int begin_frame = (int)scene_index + 1;
        int end_frame = std::min((int)scene_index + m_iscs.preload_length, (int)m_records.size());
        for (int f = begin_frame; f < end_frame; ++f)
            kickPreload(f);
    }

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
    if (time == m_last_time) {
        if (m_last_diff)
            return m_last_diff;
        else if (m_last_scene)
            return m_last_scene;
    }

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
        int si = getFrameByTime(time);
        if (interpolation) {
            auto t1 = m_records[si + 0].time;
            auto t2 = m_records[si + 1].time;

            kickPreload(si + 1);
            auto s1 = getByIndexImpl(si + 0);
            auto s2 = getByIndexImpl(si + 1);

            {
                msProfileScope("ISceneCacheImpl: [%d] lerp", (int)si);
                ScopedTimer timer;

                float t = (time - t1) / (t2 - t1);
                ret = Scene::create();
                ret->lerp(*s1, *s2, t);
                // keep a reference for s1 (s2 is not needed)
                ret->data_sources.push_back(s1);

                ret->profile_data.lerp_time = timer.elapsed();
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

void ISceneCacheImpl::preloadAll()
{
    size_t n = m_records.size();
    m_iscs.max_history = (int)n + 1;
    for (size_t i = 0; i < n; ++i)
        kickPreload(i);
}

const AnimationCurvePtr ISceneCacheImpl::getTimeCurve() const
{
    return m_time_curve;
}

const AnimationCurvePtr ISceneCacheImpl::getFrameCurve(int base_frame)
{
    // generate on the fly
    size_t scene_count = m_records.size();
    m_frame_curve = AnimationCurve::create();
    TAnimationCurve<int> curve(m_frame_curve);
    curve.resize(scene_count);
    for (size_t i = 0; i < scene_count; ++i) {
        auto& kvp = curve[i];
        kvp.time = m_records[i].time;
        kvp.value = (int)i + base_frame;
    }
    return m_frame_curve;
}


ISceneCacheFile::ISceneCacheFile(const char *path, const ISceneCacheSettings& iscs)
    : super(createStream(path, iscs), iscs)
{
}

ISceneCacheFile::StreamPtr ISceneCacheFile::createStream(const char *path, const ISceneCacheSettings& /*iscs*/)
{
    if (!path)
        return nullptr;

    auto ret = std::make_shared<std::ifstream>();
    ret->open(path, std::ios::binary);
    return *ret ? ret : nullptr;
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
