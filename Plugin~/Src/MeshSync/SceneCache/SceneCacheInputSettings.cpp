#include "pch.h"
#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"

namespace ms {

ISceneCacheSettingsBase::ISceneCacheSettingsBase()
{
    convert_scenes = 1;
    enable_diff = 1;
    generate_velocities = 0;
    preload_length = 1;
}

void ISceneCacheSettingsBase::setPreloadLength(int n)
{
    preload_length = std::max(n, 0);
    max_history = preload_length + 2;
}

} // namespace ms
