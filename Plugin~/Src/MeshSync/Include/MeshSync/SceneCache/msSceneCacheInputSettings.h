#pragma once

#include "MeshSync/SceneGraph/msSceneImportSettings.h"

namespace ms {

struct SceneCacheInputSettings {
public:
    SceneCacheInputSettings();
    explicit SceneCacheInputSettings(const SceneCacheInputSettings&) = default;

    uint32_t convert_scenes : 1;
    uint32_t enable_diff : 1;
    uint32_t generate_velocities : 1;

    SceneImportSettings sis;
};

} // namespace ms
