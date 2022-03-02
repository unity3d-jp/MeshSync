#pragma once

#include "MeshSync/msTimeRange.h" 
#include "MeshSync/SceneGraph/msScene.h"     //ScenePtr
#include "MeshSync/SceneGraph/msAnimation.h" //AnimationCurvePtr
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"

//Forward declarations
msDeclClassPtr(SceneCacheInput)

namespace ms {

class SceneCacheInput
{
public:
    virtual ~SceneCacheInput() = default;
    bool valid() const;

    int getPreloadLength() const;
    void setPreloadLength(int v);

    float getSampleRate() const;
    TimeRange getTimeRange() const;
    size_t getNumScenes() const;
    float getTime(int i) const;
    int getFrameByTime(float time) const;
    ScenePtr getByIndex(size_t i);
    ScenePtr getByTime(float t, bool lerp);
    void refresh();
    void preload(int f);
    void preloadAll();

    const AnimationCurvePtr getTimeCurve();
    const AnimationCurvePtr getFrameCurve(int base_frame);


private:
    SceneCacheInputSettings m_iscs;

};

} // namespace ms
