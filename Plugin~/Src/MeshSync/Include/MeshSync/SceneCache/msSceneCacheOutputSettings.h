#pragma once


#include "MeshSync/SceneGraph/msSceneImportSettings.h"
#include "MeshSync/SceneCache/msSceneCacheExportSettings.h"

namespace ms {

// NOT serialized in cache file
struct SceneCacheOutputSettings : SceneImportSettings
{
    SceneCacheExportSettings exportSettings;
    int maxQueueSize = 4;
    int maxSceneSegments = 8;
};

} // namespace ms
