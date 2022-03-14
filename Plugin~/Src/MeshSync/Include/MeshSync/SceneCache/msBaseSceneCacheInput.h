#pragma once

#include "MeshSync/msTimeRange.h" 
#include "MeshSync/SceneGraph/msScene.h"     //ScenePtr
#include "MeshSync/SceneGraph/msAnimation.h" //AnimationCurvePtr
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"

//Forward declarations
msDeclClassPtr(BaseSceneCacheInput)

namespace ms {

class BaseSceneCacheInput
{
public:
    BaseSceneCacheInput();
    virtual ~BaseSceneCacheInput() = default;

    inline int32_t GetPreloadLength() const;
    void SetPreloadLength(int v);
    const AnimationCurvePtr GetTimeCurve() const;

    virtual float GetSampleRateV() const = 0;
    virtual TimeRange GetTimeRangeV() const = 0;
    virtual size_t GetNumScenesV() const = 0;
    virtual float GetTimeV(int i) const = 0;
    virtual int GetFrameByTimeV(float time) const = 0;
    virtual ScenePtr LoadByFrameV(int32_t frame) = 0;
    virtual ScenePtr LoadByTimeV(float time, bool interpolation) = 0;
    virtual void RefreshV() = 0;
    virtual void PreloadV(int frame) = 0;
    virtual const AnimationCurvePtr GetFrameCurveV(int baseFrame) = 0;

protected:

    AnimationCurvePtr GetTimeCurve();

    inline int32_t GetMaxLoadedSamples() const;
    inline void SetMaxLoadedSamples(const int32_t sampleCount);
    inline const SceneCacheInputSettings& GetSettings() const;
    inline void SetSettings(const SceneCacheInputSettings& settings);

private:
    SceneCacheInputSettings m_inputSettings;
    AnimationCurvePtr m_timeCurve;

    int32_t m_maxLoadedSamples = 3;
    int32_t m_preloadLength = 1;

};

//----------------------------------------------------------------------------------------------------------------------

int32_t BaseSceneCacheInput::GetPreloadLength() const { return m_preloadLength; }

inline int32_t BaseSceneCacheInput::GetMaxLoadedSamples() const { return m_maxLoadedSamples; }
void BaseSceneCacheInput::SetMaxLoadedSamples(const int32_t sampleCount) { m_maxLoadedSamples = sampleCount; }

const SceneCacheInputSettings& BaseSceneCacheInput::GetSettings() const { return m_inputSettings; }
void BaseSceneCacheInput::SetSettings(const SceneCacheInputSettings& settings)  {m_inputSettings = settings; }


} // namespace ms
