#pragma once


#include "MeshSync/SceneGraph/msSceneImportSettings.h"
#include "MeshSync/SceneCache/msSceneCacheExportSettings.h"

namespace ms {

struct SceneCacheOutputSettings : SceneCacheExportSettings, SceneImportSettings
{
    // *not* serialized in cache file
    int max_queue_size = 4;
    int max_scene_segments = 8;
};

} // namespace ms
