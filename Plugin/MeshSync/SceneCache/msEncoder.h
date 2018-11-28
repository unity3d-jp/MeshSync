#pragma once

#include "SceneGraph/msSceneGraph.h"

namespace ms {

class BufferEncoder
{
public:
    virtual ~BufferEncoder();
    virtual void encode(RawVector<char>& dst, const RawVector<char>& src) = 0;
    virtual void decode(RawVector<char>& dst, const RawVector<char>& src) = 0;
};
using BufferEncoderPtr = std::shared_ptr<BufferEncoder>;

BufferEncoderPtr CreatePlainEncoder();
BufferEncoderPtr CreateZSTDEncoder();


class MeshEncoder
{
public:
    virtual ~MeshEncoder();
    virtual void encode(RawVector<char>& dst, const Mesh&& src) = 0;
    virtual void decode(Mesh& dst, const RawVector<char>& src) = 0;
};


enum class VertexArrayEncoding
{
    Empty,
    Plain,

    // float array encodings
    Bounded8,
    Bounded16,
    S10x3,

    // int array encodings
    I8,
    U8,
    I16,
    U16,
    U24,
};

struct MeshEncodeSettings
{
    uint32_t quantize_points : 1;
    uint32_t quantize_normals : 1;
    uint32_t quantize_tangents : 1;
    uint32_t quantize_uv : 1;
    uint32_t quantize_colors : 1;
    uint32_t quantize_velocities : 1;
};

} // namespace ms
