#pragma once


#include "MeshSync/SceneGraph/msSceneImportSettings.h"
#include "MeshSync/SceneCache/msSceneCacheExportSettings.h"

namespace ms {

// NOT serialized in cache file
struct SceneCacheOutputSettings : SceneImportSettings
{
    SceneCacheExportSettings exportSettings;
    int max_queue_size = 4;
    int max_scene_segments = 8;
};

} // namespace ms
