#pragma once
#include <future>
#include <deque>

#include "MeshSync/SceneCache/msBaseSceneCacheInput.h"
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"
#include "msSceneCacheImpl.h"

msDeclClassPtr(SceneCacheInputFile)

namespace ms {

   
class SceneCacheInputFile : public BaseSceneCacheInput
{
public:
    using StreamPtr = std::shared_ptr<std::istream>;

    ~SceneCacheInputFile() override;

    static SceneCacheInputFilePtr Open(const char *path, const SceneCacheInputSettings& iscs);
    static SceneCacheInputFile*   OpenRaw(const char *path, const SceneCacheInputSettings& iscs);

    bool IsValid() const;
    void PreloadAll();

    //Virtual
    float GetSampleRateV() const override;
    size_t GetNumScenesV() const override;
    TimeRange GetTimeRangeV() const override;
    float GetTimeV(int i) const override;
    int GetFrameByTimeV(float time) const override;
    ScenePtr GetByIndexV(size_t i) override;
    ScenePtr GetByTimeV(float time, bool interpolation) override;
    void RefreshV() override;
    void PreloadV(int frame) override;
    const AnimationCurvePtr GetFrameCurveV(int baseFrame) override;


private:
    SceneCacheInputFile() = default;
    void Init(const char *path, const SceneCacheInputSettings& iscs);
    static StreamPtr createStream(const char *path, const SceneCacheInputSettings& iscs);

    ScenePtr LoadByIndexInternal(size_t sceneIndex, bool waitPreload = true);
    ScenePtr postprocess(ScenePtr& sp, size_t scene_index);
    bool kickPreload(size_t i);
    void waitAllPreloads();
    void popHistory();

private:
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
    CacheFileHeader m_header;
    BufferEncoderPtr m_encoder;

    std::mutex m_mutex;
    std::vector<SceneRecord> m_records;
    RawVector<CacheFileEntityMeta> m_entity_meta;
    AnimationCurvePtr m_frame_curve;

    float m_last_time = -1.0f;
    int m_last_index = -1, m_last_index2 = -1;
    ScenePtr m_base_scene, m_last_scene, m_last_diff;
    std::deque<size_t> m_history;

};

} // namespace ms
