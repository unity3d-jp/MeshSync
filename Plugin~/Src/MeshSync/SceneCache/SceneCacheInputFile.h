#pragma once
#include <future>
#include <deque>

#include "MeshSync/SceneCache/msSceneCacheInput.h"
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"
#include "msSceneCacheImpl.h"

namespace ms {
   
class SceneCacheInputFile : public SceneCacheInput
{
public:
    using StreamPtr = std::shared_ptr<std::istream>;

    SceneCacheInputFile(const char *path, const SceneCacheInputSettings& iscs);

    ~SceneCacheInputFile() override;
    bool valid() const override;

    static SceneCacheInputPtr Open(const char *path, const SceneCacheInputSettings& iscs);
    static SceneCacheInput* OpenRaw(const char *path, const SceneCacheInputSettings& iscs);

    int getPreloadLength() const override;
    void setPreloadLength(int v) override;

    float getSampleRate() const override;
    size_t getNumScenes() const override;
    TimeRange getTimeRange() const override;
    float getTime(int i) const override;
    int getFrameByTime(float time) const override;
    ScenePtr getByIndex(size_t i) override;
    ScenePtr getByTime(float t, bool lerp) override;
    void refresh() override;
    void preload(int f) override;
    void preloadAll() override;

    const AnimationCurvePtr getTimeCurve() const override;
    const AnimationCurvePtr getFrameCurve(int base_frame) override;


protected:
    ScenePtr getByIndexImpl(size_t i, bool wait_preload = true);
    ScenePtr postprocess(ScenePtr& sp, size_t scene_index);
    bool kickPreload(size_t i);
    void waitAllPreloads();
    void popHistory();

private:
    void Init(StreamPtr ist, const SceneCacheInputSettings& iscs);
    static StreamPtr createStream(const char *path, const SceneCacheInputSettings& iscs);

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
    SceneCacheInputSettings m_iscs;
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

} // namespace ms
