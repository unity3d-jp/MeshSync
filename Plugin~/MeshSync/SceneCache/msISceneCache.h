#pragma once
#include "msSceneCacheImpl.h"

namespace ms {

class ISceneCacheImpl : public ISceneCache
{
public:
    using StreamPtr = std::shared_ptr<std::istream>;

    ISceneCacheImpl(StreamPtr ist, const ISceneCacheSettings& iscs);
    ~ISceneCacheImpl() override;
    bool valid() const override;

    size_t getNumScenes() const override;
    TimeRange getTimeRange() const override;
    float getTime(size_t i) const override;
    ScenePtr getByIndex(size_t i) override;
    ScenePtr getByTime(float t, bool lerp) override;

    const AnimationCurvePtr getTimeCurve() const override;

    int timeToIndex(float time) const;

protected:
    ScenePtr getByIndexImpl(size_t i);
    ScenePtr postprocess(ScenePtr& sp);

    struct SceneRecord
    {
        uint64_t pos = 0;
        uint64_t size = 0;
        ScenePtr scene;
        float time = 0.0f;
    };

    StreamPtr m_ist;
    ISceneCacheSettings m_iscs;
    CacheFileHeader m_header;

    std::mutex m_mutex;
    std::vector<SceneRecord> m_records;
    RawVector<CacheFileEntityMeta> m_entity_meta;

    AnimationCurvePtr m_time_curve;

    BufferEncoderPtr m_encoder;
    MemoryStream m_scene_buf;
    RawVector<char> m_encoded_buf, m_tmp_buf;

    float m_last_time = -1.0f;
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
