#include "pch.h"

#include "MeshSync/SceneCache/msBaseSceneCacheInput.h"

namespace ms {

BaseSceneCacheInput::BaseSceneCacheInput() 
    : m_time_curve(AnimationCurve::create())
{
    
}

int BaseSceneCacheInput::GetPreloadLength() const {
    return m_iscs.preload_length;
}

void BaseSceneCacheInput::SetPreloadLength(int v) {
    m_iscs.setPreloadLength(v);
}

//----------------------------------------------------------------------------------------------------------------------

const AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() const {
    return m_time_curve;
}


AnimationCurvePtr BaseSceneCacheInput::GetTimeCurve() {
    return m_time_curve;
}

} // namespace ms
