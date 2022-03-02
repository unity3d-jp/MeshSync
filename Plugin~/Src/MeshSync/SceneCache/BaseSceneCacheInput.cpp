#include "pch.h"

#include "MeshSync/SceneCache/msBaseSceneCacheInput.h"

namespace ms {

BaseSceneCacheInput::BaseSceneCacheInput() 
    : m_time_curve(AnimationCurve::create())
{
    
}

void BaseSceneCacheInput::SetPreloadLength(int v) {
    m_preloadLength = std::max(v, 0);
    m_maxLoadedFrames = m_preloadLength + 2;
}

//----------------------------------------------------------------------------------------------------------------------

const AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() const {
    return m_time_curve;
}


AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() {
    return m_time_curve;
}

} // namespace ms
