#pragma once
#include "msSceneCache.h"
#include "msEncoder.h"

#ifdef msEnableSceneCache
namespace ms {

struct CacheFileHeader
{
    char magic[4] = { 'M', 'S', 'S', 'C' };
    int version = msProtocolVersion;
    OSceneCacheSettingsBase oscs;
};

struct CacheFileSceneHeader
{
    uint32_t buffer_count = 0;
    float time = 0.0f;
    // flags
    uint32_t keyframe : 1;

    // uint64_t buffer_sizes[buffer_count];

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
#endif // msEnableSceneCache
