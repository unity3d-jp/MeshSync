#pragma once

#include "MeshSync/SceneGraph/msSceneImportSettings.h"

namespace ms {

struct SceneCacheInputSettings {
public:
    SceneCacheInputSettings();
    explicit SceneCacheInputSettings(const SceneCacheInputSettings&) = default;

    uint32_t convertScenes : 1;
    uint32_t enableDiff : 1;
    uint32_t generateVelocities : 1;

    SceneImportSettings importSettings;
};

} // namespace ms
