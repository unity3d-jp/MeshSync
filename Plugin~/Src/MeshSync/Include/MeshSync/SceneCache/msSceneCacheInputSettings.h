#pragma once

#include "MeshSync/SceneCache/msSceneCacheEncoding.h"
#include "MeshSync/SceneCache/msSceneCacheEncoderSettings.h"

#include "MeshSync/SceneCache/msSceneCacheInput.h"
#include "MeshSync/SceneGraph/msSceneImportSettings.h"

//Forward declarations
msDeclClassPtr(SceneCacheInput)

namespace ms {

struct ISceneCacheSettingsBase
{
    uint32_t convert_scenes : 1;
    uint32_t enable_diff : 1;
    uint32_t generate_velocities : 1;
    int max_history = 3;
    int preload_length = 1;

    SceneImportSettings sis;

    ISceneCacheSettingsBase();
    void setPreloadLength(int n);
};
struct ISceneCacheSettings : ISceneCacheSettingsBase, SceneImportSettings {};

SceneCacheInputPtr OpenISceneCacheFile(const char *path, const ISceneCacheSettings& iscs = ISceneCacheSettings());
SceneCacheInput* OpenISceneCacheFileRaw(const char *path, const ISceneCacheSettings& iscs = ISceneCacheSettings());

} // namespace ms
