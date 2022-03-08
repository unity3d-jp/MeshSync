#pragma once

#include "MeshSync/SceneCache/msSceneCacheEncoding.h"
#include "MeshSync/SceneCache/msSceneCacheEncoderSettings.h"

namespace ms {

struct SceneCacheExportSettings
{
    // serialized in cache file
    SceneCacheEncoding encoding = SceneCacheEncoding::ZSTD;
    SceneCacheEncoderSettings encoderSettings;
    float sampleRate = 30.0f; // 0.0f means 'variable sample rate'

    // flags
    uint32_t stripUnchanged : 1;
    uint32_t applyRefinement : 1;
    uint32_t flattenHierarchy : 1;
    uint32_t mergeMeshes : 1; // todo
    uint32_t stripNormals : 1;
    uint32_t stripTangents : 1;

    SceneCacheExportSettings();
};

} // namespace ms
