#pragma once
#include "msSceneCacheImpl.h"

#ifdef msEnableSceneCache
namespace ms {

class ISceneCacheImpl : public ISceneCache
{
public:
    using StreamPtr = std::shared_ptr<std::istream>;

    ISceneCacheImpl(StreamPtr ist, const ISceneCacheSettings& iscs);
    ~ISceneCacheImpl() override;
    bool valid() const override;

    float getSampleRate() const override;
    size_t getNumScenes() const override;
    TimeRange getTimeRange() const override;
    float getTime(int i) const override;
    int getFrameByTime(float time) const override;
    ScenePtr getByIndex(size_t i) override;
    ScenePtr getByTime(float t, bool lerp) override;
    void refresh() override;

    const AnimationCurvePtr getTimeCurve() const override;
    const AnimationCurvePtr getFrameCurve(int base_frame) override;

    void preloadAll();

protected:
    ScenePtr getByIndexImpl(size_t i, bool wait_preload = true);
    ScenePtr postprocess(ScenePtr& sp, size_t scene_index);
    bool kickPreload(size_t i);
    void waitAllPreloads();

    struct SceneSegment
    {
        RawVector<char> encoded_buf;
        std::future<void> task;
        ScenePtr segment;
        bool error = false;

        // profile data
        float read_time;
        float decode_time;
        uint64_t size_encoded;
        uint64_t size_decoded;
        uint64_t vertex_count;
    };

    struct SceneRecord
    {
        uint64_t pos = 0;
        uint64_t buffer_size_total = 0;
        float time = 0.0f;

        ScenePtr scene;
        std::future<void> preload;
        RawVector<uint64_t> buffer_sizes;
        std::vector<SceneSegment> segments;
    };

    StreamPtr m_ist;
    ISceneCacheSettings m_iscs;
    CacheFileHeader m_header;
    BufferEncoderPtr m_encoder;

    std::mutex m_mutex;
    std::vector<SceneRecord> m_records;
    RawVector<CacheFileEntityMeta> m_entity_meta;
    AnimationCurvePtr m_time_curve;
    AnimationCurvePtr m_frame_curve;

    float m_last_time = -1.0f;
    int m_last_index = -1, m_last_index2 = -1;
    ScenePtr m_base_scene, m_last_scene, m_last_diff;
    std::deque<size_t> m_history;
};


class ISceneCacheFile : public ISceneCacheImpl
{
using super = ISceneCacheImpl;
public:
    ISceneCacheFile(const char *path, const ISceneCacheSettings& iscs);

    static StreamPtr createStream(const char *path, const ISceneCacheSettings& iscs);
};

} // namespace ms
#endif // msEnableSceneCache
