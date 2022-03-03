#include "pch.h"
#include "SceneCacheInputFile.h"
#include "Utils/msDebug.h" //msProfileScope
#include "MeshUtils/muLog.h" //muLogError
#include "Utils/EncodingUtility.h"

namespace ms {

SceneCacheInputFile::~SceneCacheInputFile() {
    WaitAllPreloads();
}

bool SceneCacheInputFile::IsValid() const {
    return !m_records.empty();
}

//----------------------------------------------------------------------------------------------------------------------

SceneCacheInputFilePtr SceneCacheInputFile::Open(const char *path, const SceneCacheInputSettings& iscs) {
    return SceneCacheInputFilePtr(OpenRaw(path, iscs));
}

SceneCacheInputFile* SceneCacheInputFile::OpenRaw(const char *path, const SceneCacheInputSettings& iscs) {
    SceneCacheInputFile* ret = new SceneCacheInputFile();
    ret->Init(path, iscs);
    if (ret->IsValid()) {
        return ret;
    } else {
        delete ret;
        return nullptr;
    }
}

//----------------------------------------------------------------------------------------------------------------------

float SceneCacheInputFile::GetSampleRateV() const {
    return m_header.oscs.sample_rate;
}

size_t SceneCacheInputFile::GetNumScenesV() const
{
    if (!IsValid())
        return 0;
    return m_records.size();
}

TimeRange SceneCacheInputFile::GetTimeRangeV() const
{
    if (!IsValid())
        return {0.0f, 0.0f};
    return { m_records.front().time, m_records.back().time };
}

float SceneCacheInputFile::GetTimeV(int i) const
{
    if (!IsValid())
        return 0.0f;
    if (i <= 0)
        return m_records.front().time;
    if (i >= m_records.size())
        return m_records.back().time;
    return m_records[i].time;
}

int SceneCacheInputFile::GetFrameByTimeV(const float time) const
{
    if (!IsValid())
        return 0;
    auto p = std::lower_bound(m_records.begin(), m_records.end(), time, [](auto& a, float t) { return a.time < t; });
    if (p != m_records.end()) {
        int d = static_cast<int>(std::distance(m_records.begin(), p));
        return p->time == time ? d : d - 1;
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------

void SceneCacheInputFile::Init(const char *path, const SceneCacheInputSettings& iscs)
{
    m_stream = CreateStream(path, iscs);
    SetSettings(iscs);
    if (!m_stream || !(*m_stream))
        return;

    m_header.version = 0;
    m_stream->read(reinterpret_cast<char*>(&m_header), sizeof(m_header));
    if (m_header.version != msProtocolVersion)
        return;

    m_encoder = EncodingUtility::CreateEncoder(m_header.oscs.encoding, m_header.oscs.encoder_settings);
    if (!m_encoder) {
        // encoder associated with m_settings.encoding is not available
        return;
    }

    m_records.reserve(512);
    for (;;) {
        // enumerate all scene headers
        CacheFileSceneHeader sh;
        m_stream->read(reinterpret_cast<char*>(&sh), sizeof(sh));
        if (sh.buffer_count == 0) {
            // empty header is a terminator
            break;
        }
        else {
            SceneRecord rec;
            rec.time = sh.time;

            rec.bufferSizes.resize_discard(sh.buffer_count);
            m_stream->read(reinterpret_cast<char*>(rec.bufferSizes.data()), rec.bufferSizes.size_in_byte());
            rec.pos = static_cast<uint64_t>(m_stream->tellg());

            rec.bufferSizeTotal = 0;
            for (uint64_t s : rec.bufferSizes)
                rec.bufferSizeTotal += s;

            rec.segments.resize(sh.buffer_count);

            m_records.emplace_back(std::move(rec));
            m_stream->seekg(rec.bufferSizeTotal, std::ios::cur);
        }
    }

    const size_t scene_count = m_records.size();
    std::sort(m_records.begin(), m_records.end(), [](auto& a, auto& b) { return a.time < b.time; });

    TAnimationCurve<float> curve(GetTimeCurve());
    curve.resize(scene_count);
    for (size_t i = 0; i < scene_count; ++i) {
        TAnimationCurve<float>::key_t& kvp = curve[i];
        kvp.time = kvp.value = m_records[i].time;
    }

    {
        RawVector<char> encoded_buf, tmp_buf;

        // read meta data
        CacheFileMetaHeader mh;
        m_stream->read(reinterpret_cast<char*>(&mh), sizeof(mh));

        encoded_buf.resize(static_cast<size_t>(mh.size));
        m_stream->read(encoded_buf.data(), encoded_buf.size());

        m_encoder->decode(tmp_buf, encoded_buf);
        m_entityMeta.resize_discard(tmp_buf.size() / sizeof(CacheFileEntityMeta));
        tmp_buf.copy_to(reinterpret_cast<char*>(m_entityMeta.data()));
    }

    if (m_header.oscs.strip_unchanged)
        m_baseScene = LoadByIndexInternal(0);

    //PreloadAll(); // for test
}


SceneCacheInputFile::StreamPtr SceneCacheInputFile::CreateStream(const char *path, const SceneCacheInputSettings& /*iscs*/)
{
    if (!path)
        return nullptr;

    std::shared_ptr<std::basic_ifstream<char>> ret = std::make_shared<std::ifstream>();
    ret->open(path, std::ios::binary);
    return *ret ? ret : nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

// thread safe
ScenePtr SceneCacheInputFile::LoadByIndexInternal(size_t sceneIndex, bool waitPreload)
{
    if (!IsValid() || sceneIndex >= m_records.size())
        return nullptr;

    SceneRecord& rec = m_records[sceneIndex];
    if (waitPreload && rec.preload.valid()) {
        // wait preload
        rec.preload.wait();
        rec.preload = {};
    }

    ScenePtr& ret = rec.scene;
    if (ret)
        return ret; // already loaded

    const mu::nanosec load_begin = mu::Now();

    const size_t seg_count = rec.bufferSizes.size();
    rec.segments.resize(seg_count);

    {
        // get exclusive file access
        std::unique_lock<std::mutex> lock(m_mutex);

        m_stream->seekg(rec.pos, std::ios::beg);
        for (size_t si = 0; si < seg_count; ++si) {
            SceneSegment& seg = rec.segments[si];
            seg.encodedSize = rec.bufferSizes[si];

            // read segment
            {
                msProfileScope("SceneCacheInputFile: [%d] read segment (%d - %u byte)", (int)sceneIndex, (int)si, (uint32_t)seg.encodedSize);
                mu::ScopedTimer timer;

                seg.encodedBuf.resize(static_cast<size_t>(seg.encodedSize));
                m_stream->read(seg.encodedBuf.data(), seg.encodedBuf.size());

                seg.readTime = timer.elapsed();
            }

            // launch async decode
            seg.task = std::async(std::launch::async, [this, &seg, sceneIndex, si]() {
                msProfileScope("SceneCacheInputFile: [%d] decode segment (%d)", (int)sceneIndex, (int)si);
                mu::ScopedTimer timer;

                RawVector<char> tmp_buf;
                m_encoder->decode(tmp_buf, seg.encodedBuf);
                seg.decodedSize = tmp_buf.size();

                std::shared_ptr<Scene> ret = Scene::create();
                mu::MemoryStream scene_buf(std::move(tmp_buf));
                try {
                    ret->deserialize(scene_buf);

                    // keep scene buffer alive. Meshes will use it as vertex buffers
                    ret->scene_buffers.push_back(scene_buf.moveBuffer());
                    seg.segment = ret;

                    // count vertices
                    seg.vertexCount = 0;
                    for (std::vector<std::shared_ptr<Transform>>::value_type& e : seg.segment->entities)
                        seg.vertexCount += e->vertexCount();
                }
                catch (std::runtime_error& e) {
                    muLogError("exception: %s\n", e.what());
                    ret = nullptr;
                    seg.error = true;
                }

                seg.decodeTime = timer.elapsed();
            });
        }
    }

    // concat segmented scenes
    for (size_t si = 0; si < seg_count; ++si) {
        SceneSegment& seg = rec.segments[si];
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
        SceneProfileData& prof = ret->profile_data;
        prof = {};
        prof.load_time = mu::NS2MS(mu::Now() - load_begin);
        for (std::vector<SceneSegment>::value_type& seg : rec.segments) {
            prof.size_encoded += seg.encodedSize;
            prof.size_decoded += seg.decodedSize;
            prof.read_time += seg.readTime;
            prof.decode_time += seg.decodeTime;
            prof.vertex_count += seg.vertexCount;
        }

        {
            msProfileScope("SceneCacheInputFile: [%d] merge & import", static_cast<int>(sceneIndex));
            mu::ScopedTimer timer;

            if (m_header.oscs.strip_unchanged && m_baseScene) {
                // set cache flags
                size_t n = ret->entities.size();
                if (m_entityMeta.size() == n) {
                    mu::enumerate(m_entityMeta, ret->entities, [](auto& meta, auto& e) {
                        if (meta.id == e->id) {
                            e->cache_flags.constant = meta.constant;
                            e->cache_flags.constant_topology = meta.constant_topology;
                        }
                        });
                }

                // merge
                ret->merge(*m_baseScene);
            }

            // do import
            const SceneCacheInputSettings& settings = GetSettings();
            ret->import(settings.sis);

            prof.setup_time = timer.elapsed();
        }
    }
    rec.segments.clear();

    // push & pop history
    if (!m_header.oscs.strip_unchanged || sceneIndex != 0) {
        m_history.push_back(sceneIndex);
        PopOverflowedSamples();
    }
    return ret;
}

ScenePtr SceneCacheInputFile::PostProcess(ScenePtr& sp, const size_t sceneIndex)
{
    if (!sp)
        return sp;

    ScenePtr ret;

    // m_lastScene and m_lastDiff keep reference counts and keep scenes alive.
    // (plugin APIs return raw scene pointers. someone needs to keep its reference counts)
    const SceneCacheInputSettings& settings = GetSettings();
    if (m_lastScene && (settings.enable_diff && m_header.oscs.strip_unchanged)) {
        msProfileScope("SceneCacheInputFile: [%d] diff", static_cast<int>(sceneIndex));
        m_lastDiff = Scene::create();
        m_lastDiff->diff(*sp, *m_lastScene);
        m_lastScene = sp;
        ret = m_lastDiff;
    }
    else {
        m_lastDiff = nullptr;
        m_lastScene = sp;
        ret = sp;
    }

    // kick preload
    PreloadV(static_cast<int>(sceneIndex));

    return ret;
}

bool SceneCacheInputFile::KickPreload(size_t i)
{
    SceneRecord& rec = m_records[i];
    if (rec.scene || rec.preload.valid())
        return false; // already loaded or loading

    rec.preload = std::async(std::launch::async, [this, i]() { LoadByIndexInternal(i, false); });
    return true;
}

void SceneCacheInputFile::WaitAllPreloads()
{
    for (std::vector<SceneRecord>::value_type& rec : m_records) {
        if (rec.preload.valid()) {
            rec.preload.wait();
            rec.preload = {};
        }
    }
}

ScenePtr SceneCacheInputFile::LoadByIndexV(size_t i)
{
    if (!IsValid())
        return nullptr;

    ScenePtr ret = LoadByIndexInternal(i);
    return PostProcess(ret, i);
}

ScenePtr SceneCacheInputFile::LoadByTimeV(const float time, const bool interpolation)
{
    if (!IsValid())
        return nullptr;
    if (time == m_lastTime) {
        if (m_lastDiff)
            return m_lastDiff;
        else if (m_lastScene)
            return m_lastScene;
    }

    ScenePtr ret;

    const int scene_count = static_cast<int>(m_records.size());

    const TimeRange time_range = GetTimeRangeV();
    if (time <= time_range.start) {
        const int si = 0;
        if ((!interpolation && m_lastIndex == si) ||
            (m_lastIndex == si && m_lastIndex2 == si))
            return nullptr;

        m_lastIndex = m_lastIndex2 = si;
        ret = LoadByIndexInternal(si);
    }
    else if (time >= time_range.end) {
        const int si =  scene_count - 1;
        if ((!interpolation && m_lastIndex == si) ||
            (m_lastIndex == si && m_lastIndex2 == si))
            return nullptr;

        m_lastIndex = m_lastIndex2 = si;
        ret = LoadByIndexInternal(si);
    }
    else {
        const int si = GetFrameByTimeV(time);
        if (interpolation) {
            const float t1 = m_records[si + 0].time;
            const float t2 = m_records[si + 1].time;

            KickPreload(si + 1);
            const ScenePtr s1 = LoadByIndexInternal(si + 0);
            const ScenePtr s2 = LoadByIndexInternal(si + 1);

            {
                msProfileScope("SceneCacheInputFile: [%d] lerp", (int)si);
                mu::ScopedTimer timer;

                float t = (time - t1) / (t2 - t1);
                ret = Scene::create();
                ret->lerp(*s1, *s2, t);
                // keep a reference for s1 (s2 is not needed)
                ret->data_sources.push_back(s1);

                ret->profile_data.lerp_time = timer.elapsed();
            }

            m_lastIndex = si;
            m_lastIndex2 = si + 1;
        }
        else {
            if (si == m_lastIndex)
                return nullptr;
            ret = LoadByIndexInternal(si);

            m_lastIndex = m_lastIndex2 = si;
        }
    }
    m_lastTime = time;
    return PostProcess(ret, m_lastIndex2);
}

void SceneCacheInputFile::RefreshV()
{
    m_lastIndex = m_lastIndex2  = -1;
    m_lastTime = -1.0f;
    m_lastScene = nullptr;
    m_lastDiff = nullptr;
}

void SceneCacheInputFile::PreloadV(const int frame)
{
    // kick preload
    const int32_t preloadLength = GetPreloadLength();
    if (preloadLength> 0 && frame + 1 < m_records.size()) {
        const int begin_frame = frame + 1;
        const int end_frame = std::min(frame + preloadLength, static_cast<int>(m_records.size()));
        for (int f = begin_frame; f < end_frame; ++f)
            KickPreload(f);
    }
    PopOverflowedSamples();
}

void SceneCacheInputFile::PreloadAll()
{
    const size_t n = m_records.size();
    SetMaxLoadedSamples(static_cast<int>(n) + 1);
    for (size_t i = 0; i < n; ++i)
        KickPreload(i);
}

void SceneCacheInputFile::PopOverflowedSamples()
{
    const int32_t maxSamples = GetMaxLoadedSamples();
    while (m_history.size() > maxSamples) {
        m_records[m_history.front()].scene.reset();
        m_history.pop_front();
    }
}

const AnimationCurvePtr SceneCacheInputFile::GetFrameCurveV(const int baseFrame)
{
    // generate on the fly
    const size_t sceneCount = m_records.size();
    m_frameCurve = AnimationCurve::create();
    TAnimationCurve<int> curve(m_frameCurve);
    curve.resize(sceneCount);
    for (size_t i = 0; i < sceneCount; ++i) {
        TAnimationCurve<int>::key_t& kvp = curve[i];
        kvp.time = m_records[i].time;
        kvp.value = static_cast<int>(i) + baseFrame;
    }
    return m_frameCurve;
}

} // namespace ms
