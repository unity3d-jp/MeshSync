#pragma once

#include "MeshSync/SceneCache/msSceneCacheSettings.h" //TimeRange
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msAnimation.h"

//Forward declarations
msDeclClassPtr(Scene)
msDeclClassPtr(SceneCacheInput)

namespace ms {

class SceneCacheInput
{
public:
    virtual ~SceneCacheInput() = default;
    virtual bool valid() const = 0;

    virtual int getPreloadLength() const = 0;
    virtual void setPreloadLength(int v) = 0;

    virtual float getSampleRate() const = 0;
    virtual TimeRange getTimeRange() const = 0;
    virtual size_t getNumScenes() const = 0;
    virtual float getTime(int i) const = 0;
    virtual int getFrameByTime(float time) const = 0;
    virtual ScenePtr getByIndex(size_t i) = 0;
    virtual ScenePtr getByTime(float t, bool lerp) = 0;
    virtual void refresh() = 0;
    virtual void preload(int f) = 0;
    virtual void preloadAll() = 0;

    virtual const AnimationCurvePtr getTimeCurve() const = 0;
    virtual const AnimationCurvePtr getFrameCurve(int base_frame) = 0;
};

} // namespace ms
