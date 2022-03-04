#pragma once

#include "MeshSync/SceneGraph/msScene.h"

msDeclClassPtr(BufferEncoder);

namespace ms {

class BufferEncoder
{
public:
    virtual ~BufferEncoder() = default;
    virtual void EncodeV(RawVector<char>& dst, const RawVector<char>& src) = 0;
    virtual void DecodeV(RawVector<char>& dst, const RawVector<char>& src) = 0;
};

BufferEncoderPtr CreatePlainEncoder();
BufferEncoderPtr CreateZSTDEncoder(int compression_level);

} // namespace ms
