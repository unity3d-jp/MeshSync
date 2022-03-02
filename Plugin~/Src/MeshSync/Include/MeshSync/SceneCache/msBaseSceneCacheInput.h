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

    int GetPreloadLength() const;
    void SetPreloadLength(int v);
    const AnimationCurvePtr GetTimeCurve() const;

    virtual float GetSampleRateV() const = 0;
    virtual TimeRange GetTimeRangeV() const = 0;
    virtual size_t GetNumScenesV() const = 0;
    virtual float GetTimeV(int i) const = 0;
    virtual int GetFrameByTimeV(float time) const = 0;
    virtual ScenePtr GetByIndexV(size_t i) = 0;
    virtual ScenePtr GetByTimeV(float time, bool interpolation) = 0;
    virtual void RefreshV() = 0;
    virtual void PreloadV(int frame) = 0;
    virtual const AnimationCurvePtr GetFrameCurveV(int baseFrame) = 0;

protected:

    AnimationCurvePtr GetTimeCurve();

private:
    SceneCacheInputSettings m_iscs;
    AnimationCurvePtr m_time_curve;
};

} // namespace ms
