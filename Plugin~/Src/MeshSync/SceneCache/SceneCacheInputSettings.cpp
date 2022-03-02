#include "pch.h"
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"

namespace ms {

SceneCacheInputSettings::SceneCacheInputSettings()
    : convert_scenes(1)
    , enable_diff (1)
    , generate_velocities(0)
{
}

} // namespace ms
