#pragma once

#include "MeshSync/SceneGraph/msSceneImportSettings.h"

namespace ms {

struct ISceneCacheSettingsBase
{
    uint32_t convert_scenes : 1;
    uint32_t enable_diff : 1;
    uint32_t generate_velocities : 1;

    SceneImportSettings sis;

    ISceneCacheSettingsBase();
};
struct SceneCacheInputSettings : ISceneCacheSettingsBase, SceneImportSettings{
public:
    SceneCacheInputSettings() = default;
    explicit SceneCacheInputSettings(const SceneCacheInputSettings&) = default;
};


} // namespace ms
