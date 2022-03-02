#include "pch.h"

#include "MeshSync/SceneCache/msSceneCacheInput.h"

namespace ms {

int SceneCacheInput::getPreloadLength() const {
    return m_iscs.preload_length;
}

void SceneCacheInput::setPreloadLength(int v) {
    m_iscs.setPreloadLength(v);
}


} // namespace ms
