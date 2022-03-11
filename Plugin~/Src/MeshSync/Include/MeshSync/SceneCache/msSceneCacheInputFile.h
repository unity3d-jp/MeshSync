#pragma once
#include <future>
#include <deque>

#include "MeshSync/SceneCache/msBaseSceneCacheInput.h"
#include "MeshSync/SceneCache/msCacheFileHeader.h"
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"


msDeclClassPtr(SceneCacheInputFile)
msDeclClassPtr(BufferEncoder)

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
    ScenePtr LoadByFrameV(int32_t frame) override;
    ScenePtr LoadByTimeV(float time, bool interpolation) override;
    void RefreshV() override;
    void PreloadV(int frame) override;
    const AnimationCurvePtr GetFrameCurveV(int baseFrame) override;


private:
    SceneCacheInputFile() = default;
    void Init(const char *path, const SceneCacheInputSettings& iscs);
    static StreamPtr CreateStream(const char *path, const SceneCacheInputSettings& iscs);

    ScenePtr LoadByFrameInternal(size_t sceneIndex, bool waitPreload = true);
    ScenePtr PostProcess(ScenePtr& sp, size_t sceneIndex);
    bool KickPreload(size_t i);
    void WaitAllPreloads();
    void PopOverflowedSamples();

private:
    struct SceneSegment
    {
        RawVector<char> encodedBuf;
        std::future<void> task;
        ScenePtr segment;
        bool error = false;

        // profile data
        float readTime;
        float decodeTime;
        uint64_t encodedSize;
        uint64_t decodedSize;
        uint64_t vertexCount;
    };

    struct SceneRecord
    {
        uint64_t pos = 0;
        uint64_t bufferSizeTotal = 0;
        float time = 0.0f;

        ScenePtr scene;
        std::future<void> preload;
        RawVector<uint64_t> bufferSizes;
        std::vector<SceneSegment> segments;
    };

    StreamPtr m_stream;
    CacheFileHeader m_header;
    BufferEncoderPtr m_encoder;

    std::mutex m_mutex;
    std::vector<SceneRecord> m_records;
    RawVector<CacheFileEntityMeta> m_entityMeta;
    AnimationCurvePtr m_frameCurve;

    float m_lastTime = -1.0f;
    int m_loadedFrame0 = -1, m_loadedFrame1 = -1;
    ScenePtr m_baseScene, m_lastScene, m_lastDiff;
    std::deque<size_t> m_history;

};

} // namespace ms
