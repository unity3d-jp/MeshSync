#pragma once

#include "MeshSync/SceneCache/msSceneCacheExportSettings.h"
#include "MeshSync/msConfig.h"

namespace ms {

struct CacheFileHeader
{
    char magic[4] = { 'M', 'S', 'S', 'C' };
    int version = msProtocolVersion;
    SceneCacheExportSettings exportSettings;
};

struct CacheFileSceneHeader
{
    uint32_t bufferCount = 0;
    float time = 0.0f;
    // flags
    uint32_t keyframe : 1;

    // uint64_t buffer_sizes[bufferCount];

    static CacheFileSceneHeader terminator() { return CacheFileSceneHeader(); }
};

struct CacheFileMetaHeader
{
    uint64_t size = 0;
};

struct CacheFileEntityMeta
{
    int id = 0;
    uint32_t type : 4; // EntityType
    uint32_t constant : 1;
    uint32_t constantTopology : 1;
};

} // namespace ms
