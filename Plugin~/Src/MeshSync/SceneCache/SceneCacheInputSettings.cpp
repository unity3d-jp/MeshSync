#include "pch.h"
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"

namespace ms {

SceneCacheInputSettings::SceneCacheInputSettings()
    : convertScenes(1)
    , enableDiff (1)
    , generateVelocities(0)
{
}

} // namespace ms
