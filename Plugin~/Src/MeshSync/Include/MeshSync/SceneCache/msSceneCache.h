#pragma once

#include "MeshSync/SceneCache/msSceneCacheSettings.h" //TimeRange
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msAnimation.h"

//Forward declarations
msDeclClassPtr(Scene)
msDeclClassPtr(ISceneCache)
msDeclClassPtr(OSceneCache)

namespace ms {

class OSceneCache
{
public:
    virtual ~OSceneCache() {}
    virtual bool valid() const = 0;

    // note:
    // *scene will be modified* if scene optimization options (strip_unchanged, apply_refinement, etc) are enabled.
    // pass cloned scene (Scene::clone()) if you need to keep source scenes intact.
    virtual void addScene(ScenePtr scene, float time) = 0;

    virtual void flush() = 0;
    virtual bool isWriting() = 0;
    virtual int getSceneCountWritten() const = 0;
    virtual int getSceneCountInQueue() const = 0;
};

class ISceneCache
{
public:
    virtual ~ISceneCache() {}
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
