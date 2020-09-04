#include "pch.h"
#include "MeshSync/SceneCache/msSceneCache.h"
#include "msSceneCacheImpl.h"

#ifdef msEnableSceneCache
namespace ms {

static_assert(sizeof(CacheFileEntityMeta) == 8, "");

OSceneCacheSettingsBase::OSceneCacheSettingsBase()
{
    encoding = SceneCacheEncoding::ZSTD;
    encoder_settings.zstd.compression_level = GetZSTDDefaultCompressionLevel();

    strip_unchanged = 1;
    apply_refinement = 1;
    flatten_hierarchy = 0;
    merge_meshes = 0;
    strip_normals = 0;
    strip_tangents = 0;
}

ISceneCacheSettingsBase::ISceneCacheSettingsBase()
{
    convert_scenes = 1;
    enable_diff = 1;
    generate_velocities = 0;
    preload_length = 1;
}

void ISceneCacheSettingsBase::setPreloadLength(int n)
{
    preload_length = std::max(n, 0);
    max_history = preload_length + 2;
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
#endif // msEnableSceneCache
