#include "pch.h"
#include "msSceneCache.h"
#include "msSceneCacheImpl.h"
#include "msMisc.h"

namespace ms {

static_assert(sizeof(CacheFileEntityMeta) == 8, "");

OSceneCacheSettingsBase::OSceneCacheSettingsBase()
{
    encoding = SceneCacheEncoding::ZSTD;
    encoder_settings.zstd.compression_level = GetZSTDDefaultCompressionLevel();

    strip_unchanged = 1;
    apply_refinement = 1;
    flatten_hierarchy = 1;
    merge_meshes = 0;
    strip_normals = 0;
    strip_tangents = 0;
}

ISceneCacheSettingsBase::ISceneCacheSettingsBase()
{
    convert_scenes = 1;
    enable_diff = 1;
    preload_scenes = 1;
    preload_entire_file = 0;
    generate_velocities = 0;
}

BufferEncoderPtr CreateEncoder(SceneCacheEncoding encoding, const SceneCacheEncoderSettings& settings)
{
    BufferEncoderPtr ret;
    switch (encoding) {
    case SceneCacheEncoding::ZSTD: ret = CreateZSTDEncoder(settings.zstd.compression_level); break;
    default: break;
    }
    return ret;
}

} // namespace ms
