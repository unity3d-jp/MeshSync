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
    virtual ScenePtr GetByIndexV(size_t i) = 0;
    virtual ScenePtr GetByTimeV(float time, bool interpolation) = 0;
    virtual void RefreshV() = 0;
    virtual void PreloadV(int frame) = 0;
    virtual const AnimationCurvePtr GetFrameCurveV(int baseFrame) = 0;

protected:

    AnimationCurvePtr GetTimeCurve();

    inline int32_t GetMaxHistory() const;
    inline void SetMaxHistory(const int32_t count);

private:
    SceneCacheInputSettings m_iscs;
    AnimationCurvePtr m_time_curve;

    int32_t max_history = 3;
    int32_t preload_length = 1;

};

//----------------------------------------------------------------------------------------------------------------------

int32_t BaseSceneCacheInput::GetPreloadLength() const { return preload_length; }

inline int32_t BaseSceneCacheInput::GetMaxHistory() const { return max_history; }
void BaseSceneCacheInput::SetMaxHistory(const int32_t count) { max_history = count; }


} // namespace ms
