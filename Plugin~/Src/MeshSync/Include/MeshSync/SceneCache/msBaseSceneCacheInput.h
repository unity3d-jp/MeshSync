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

    virtual float getSampleRate() const = 0;
    virtual TimeRange getTimeRange() const = 0;
    virtual size_t getNumScenes() const = 0;
    virtual float getTime(int i) const = 0;
    virtual int getFrameByTime(float time) const = 0;
    virtual ScenePtr getByIndex(size_t i) = 0;
    virtual ScenePtr getByTime(float t, bool lerp) = 0;
    virtual void refresh() = 0;
    virtual void preload(int f) = 0;    
    virtual const AnimationCurvePtr getFrameCurve(int base_frame) = 0;

protected:

    AnimationCurvePtr GetTimeCurve();

private:
    SceneCacheInputSettings m_iscs;
    AnimationCurvePtr m_time_curve;
};

} // namespace ms
