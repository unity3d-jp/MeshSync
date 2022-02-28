#pragma once

#include "MeshSync/SceneCache/msSceneCacheSettings.h" //TimeRange
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msAnimation.h"

//Forward declarations
msDeclClassPtr(Scene)
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

} // namespace ms
