#include "pch.h"
#include "MeshSync/SceneCache/msSceneCacheOutputSettings.h"
#include "MeshSync/Utils/EncodingUtility.h"
#include "msSceneCacheImpl.h"

namespace ms {

static_assert(sizeof(CacheFileEntityMeta) == 8, "");

OSceneCacheSettingsBase::OSceneCacheSettingsBase()
{
    encoding = SceneCacheEncoding::ZSTD;
    encoder_settings.zstd.compression_level = EncodingUtility::GetZSTDDefaultCompressionLevel();

    strip_unchanged = 1;
    apply_refinement = 1;
    flatten_hierarchy = 0;
    merge_meshes = 0;
    strip_normals = 0;
    strip_tangents = 0;
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
