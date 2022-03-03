#include "pch.h"

#include "MeshSync/SceneCache/msBaseSceneCacheInput.h"

namespace ms {

BaseSceneCacheInput::BaseSceneCacheInput() 
    : m_timeCurve(AnimationCurve::create())
{
    
}

void BaseSceneCacheInput::SetPreloadLength(int v) {
    m_preloadLength = std::max(v, 0);
    m_maxLoadedSamples = m_preloadLength + 2;
}

//----------------------------------------------------------------------------------------------------------------------

const AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() const {
    return m_timeCurve;
}


AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() {
    return m_timeCurve;
}

} // namespace ms
