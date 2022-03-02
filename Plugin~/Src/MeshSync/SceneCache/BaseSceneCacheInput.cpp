#include "pch.h"

#include "MeshSync/SceneCache/msBaseSceneCacheInput.h"

namespace ms {

BaseSceneCacheInput::BaseSceneCacheInput() 
    : m_time_curve(AnimationCurve::create())
{
    
}

void BaseSceneCacheInput::SetPreloadLength(int v) {
    preload_length = std::max(v, 0);
    max_history = preload_length + 2;
}

//----------------------------------------------------------------------------------------------------------------------

const AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() const {
    return m_time_curve;
}


AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() {
    return m_time_curve;
}

} // namespace ms
