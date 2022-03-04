#include "pch.h"
#include "zstd.h"

#include "MeshSync/Utils/EncodingUtility.h"

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

} // namespace ms
