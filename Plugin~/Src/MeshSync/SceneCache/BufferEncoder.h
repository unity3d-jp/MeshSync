#pragma once

#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneCache/msSceneCacheEncoding.h"

msDeclClassPtr(BufferEncoder);

namespace ms {

union SceneCacheEncoderSettings;

class BufferEncoder {
public:
    virtual ~BufferEncoder() = default;
    virtual void EncodeV(RawVector<char>& dst, const RawVector<char>& src) = 0;
    virtual void DecodeV(RawVector<char>& dst, const RawVector<char>& src) = 0;

    static BufferEncoderPtr CreateEncoder(ms::SceneCacheEncoding encoding, const ms::SceneCacheEncoderSettings& settings);

};


} // namespace ms
