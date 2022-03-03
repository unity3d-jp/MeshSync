#include "pch.h"
#include "zstd.h"

#include "MeshSync/Utils/EncodingUtility.h"
#include "MeshSync/SceneCache/msSceneCacheEncoderSettings.h"


namespace ms {

std::tuple<int, int> EncodingUtility::GetZSTDCompressionLevelRange()
{
    return{ ZSTD_minCLevel(), ZSTD_maxCLevel() };
}

int EncodingUtility::ClampZSTDCompressionLevel(int v)
{
    return mu::clamp(v, 0, ZSTD_maxCLevel());
}

int EncodingUtility::GetZSTDDefaultCompressionLevel()
{
    return ZSTD_CLEVEL_DEFAULT;
}

BufferEncoderPtr EncodingUtility::CreateEncoder(SceneCacheEncoding encoding, const SceneCacheEncoderSettings& settings)
{
    BufferEncoderPtr ret;
    switch (encoding) {
        case SceneCacheEncoding::ZSTD: ret = CreateZSTDEncoder(settings.zstd.compression_level); break;
        default: break;
    }
    return ret;
}

} // namespace ms
