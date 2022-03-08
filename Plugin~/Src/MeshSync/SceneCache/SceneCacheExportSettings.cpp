#include "pch.h"
#include "MeshSync/SceneCache/msSceneCacheExportSettings.h"

#include "Utils/EncodingUtility.h"

namespace ms {

SceneCacheExportSettings::SceneCacheExportSettings()
{
    encoding = SceneCacheEncoding::ZSTD;
    encoder_settings.zstd.compressionLevel = EncodingUtility::GetZSTDDefaultCompressionLevel();

    strip_unchanged = 1;
    apply_refinement = 1;
    flatten_hierarchy = 0;
    merge_meshes = 0;
    strip_normals = 0;
    strip_tangents = 0;
}


} // namespace ms
