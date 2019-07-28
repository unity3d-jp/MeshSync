#pragma once
#include "msSceneCache.h"
#include "msEncoder.h"

namespace ms {

struct CacheFileHeader
{
    char magic[4] = { 'M', 'S', 'S', 'C' };
    int version = msProtocolVersion;
    OSceneCacheSettingsBase oscs;
};

struct CacheFileSceneHeader
{
    uint64_t size = 0;
    float time = 0.0f;
    uint32_t keyframe : 1;

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
    uint32_t constant_topology : 1;
};


BufferEncoderPtr CreateEncoder(SceneCacheEncoding encoding, const SceneCacheEncoderSettings& settings);

} // namespace ms
