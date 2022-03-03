#pragma once

#include "MeshSync/SceneCache/msEncoder.h"
#include "MeshSync/SceneCache/msSceneCacheEncoding.h"

namespace ms {
    union SceneCacheEncoderSettings;

    class EncodingUtility {
public:
    static std::tuple<int, int> GetZSTDCompressionLevelRange();
    static int ClampZSTDCompressionLevel(int v);
    static int GetZSTDDefaultCompressionLevel();

    static BufferEncoderPtr CreateEncoder(ms::SceneCacheEncoding encoding, const ms::SceneCacheEncoderSettings& settings);

};

} //end namespace ms
