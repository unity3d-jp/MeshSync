#include "pch.h"
#include "MeshSync/SceneCache/msSceneCacheExportSettings.h"

#include "Utils/EncodingUtility.h"

namespace ms {

SceneCacheExportSettings::SceneCacheExportSettings()
{
    encoding = SceneCacheEncoding::ZSTD;
    encoderSettings.zstd.compressionLevel = EncodingUtility::GetZSTDDefaultCompressionLevel();

    stripUnchanged = 1;
    applyRefinement = 1;
    flattenHierarchy = 0;
    mergeMeshes = 0;
    stripNormals = 0;
    stripTangents = 0;
}


} // namespace ms
