#include "pch.h"

#include "MeshSync/SceneCache/msSceneCacheInput.h"

namespace ms {

SceneCacheInput::SceneCacheInput() 
    : m_time_curve(AnimationCurve::create())
{
    
}

int SceneCacheInput::getPreloadLength() const {
    return m_iscs.preload_length;
}

void SceneCacheInput::setPreloadLength(int v) {
    m_iscs.setPreloadLength(v);
}

//----------------------------------------------------------------------------------------------------------------------

const AnimationCurvePtr SceneCacheInput::getTimeCurve() const {
    return m_time_curve;
}


AnimationCurvePtr SceneCacheInput::GetTimeCurve() {
    return m_time_curve;
}

} // namespace ms
