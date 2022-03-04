#pragma once

#include "MeshSync/SceneGraph/msScene.h"

msDeclClassPtr(BufferEncoder);
msDeclClassPtr(Mesh);

namespace ms {

class BufferEncoder
{
public:
    virtual ~BufferEncoder();
    virtual void encode(RawVector<char>& dst, const RawVector<char>& src) = 0;
    virtual void decode(RawVector<char>& dst, const RawVector<char>& src) = 0;
};

BufferEncoderPtr CreatePlainEncoder();
BufferEncoderPtr CreateZSTDEncoder(int compression_level);

} // namespace ms
