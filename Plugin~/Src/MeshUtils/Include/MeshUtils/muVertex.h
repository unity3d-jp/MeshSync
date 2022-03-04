#pragma once

namespace mu {

struct Weights1
{
    float   weight;
    int     index;

    // assume 'this' is an element of array
    void copy_to(Weights1 *dst, int n)
    {
        // avoid std::copy() because it is way slower than memcpy on some compilers...
        memcpy(dst, this, sizeof(Weights1)*n);
    }

    // assume 'this' is an element of array
    float normalize(int n)
    {
        float total = 0.0f;

        auto *weights = this;
        for (int i = 0; i < n; ++i)
            total += weights[i].weight;

        if (total != 0.0f) {
            float rcp_total = 1.0f / total;
            for (int i = 0; i < n; ++i)
                weights[i].weight *= rcp_total;
        }
        return total;
    }
};

template<int N>
struct Weights
{
    float   weights[N] = {};
    int     indices[N] = {};

    float normalize()
    {
        float total = 0.0f;
        for (auto w : weights)
            total += w;

        if (total != 0.0f) {
            float rcp_total = 1.0f / total;
            for (auto& w : weights)
                w *= rcp_total;
        }
        return total;
    }
};
using Weights4 = Weights<4>;


// vertex interleave

enum class VertexFormat
{
    Unknown,
    V3N3,
    V3N3C4,
    V3N3U2,
    V3N3C4U2,
    V3N3U2T4,
    V3N3C4U2T4,
};

struct vertex_v3n3;
struct vertex_v3n3_arrays;
struct vertex_v3n3c4;
struct vertex_v3n3c4_arrays;
struct vertex_v3n3u2;
struct vertex_v3n3u2_arrays;
struct vertex_v3n3c4u2;
struct vertex_v3n3c4u2_arrays;
struct vertex_v3n3u2t4;
struct vertex_v3n3u2t4_arrays;
struct vertex_v3n3c4u2t4;
struct vertex_v3n3c4u2t4_arrays;

#define DefTraits(T, ID)\
    static const VertexFormat tid = VertexFormat::ID;\
    using vertex_t = T;\
    using arrays_t = T##_arrays;


struct vertex_v3n3_arrays
{
    DefTraits(vertex_v3n3, V3N3)
    const float3 *points;
    const float3 *normals;
};
struct vertex_v3n3
{
    DefTraits(vertex_v3n3, V3N3)
    float3 p;
    float3 n;
};

struct vertex_v3n3c4_arrays
{
    DefTraits(vertex_v3n3c4, V3N3C4)
    const float3 *points;
    const float3 *normals;
    const float4 *colors;
};
struct vertex_v3n3c4
{
    DefTraits(vertex_v3n3c4, V3N3C4)
    float3 p;
    float3 n;
    float4 c;
};


struct vertex_v3n3u2_arrays
{
    DefTraits(vertex_v3n3u2, V3N3U2)
    const float3 *points;
    const float3 *normals;
    const float2 *uvs;
};
struct vertex_v3n3u2
{
    DefTraits(vertex_v3n3u2, V3N3U2)
    float3 p;
    float3 n;
    float2 u;
};

struct vertex_v3n3c4u2_arrays
{
    DefTraits(vertex_v3n3c4u2, V3N3C4U2)
    const float3 *points;
    const float3 *normals;
    const float4 *colors;
    const float2 *uvs;
};
struct vertex_v3n3c4u2
{
    DefTraits(vertex_v3n3c4u2, V3N3C4U2)
    float3 p;
    float3 n;
    float4 c;
    float2 u;
};

struct vertex_v3n3u2t4_arrays
{
    DefTraits(vertex_v3n3u2t4, V3N3U2T4)
    const float3 *points;
    const float3 *normals;
    const float2 *uvs;
    const float4 *tangents;
};
struct vertex_v3n3u2t4
{
    DefTraits(vertex_v3n3u2t4, V3N3U2T4)
    float3 p;
    float3 n;
    float2 u;
    float4 t;
};

struct vertex_v3n3c4u2t4_arrays
{
    DefTraits(vertex_v3n3u2t4, V3N3C4U2T4)
    const float3 *points;
    const float3 *normals;
    const float4 *colors;
    const float2 *uvs;
    const float4 *tangents;
};
struct vertex_v3n3c4u2t4
{
    DefTraits(vertex_v3n3c4u2t4, V3N3C4U2T4)
    float3 p;
    float3 n;
    float4 c;
    float2 u;
    float4 t;
};
#undef DefTraits

VertexFormat GuessVertexFormat(
    const float3 *points,
    const float3 *normals,
    const float4 *colors,
    const float2 *uvs,
    const float4 *tangents
);

size_t GetVertexSize(VertexFormat format);

void Interleave(void *dst, VertexFormat format, size_t num,
    const float3 *points,
    const float3 *normals,
    const float4 *colors,
    const float2 *uvs,
    const float4 *tangents
);

template<class VertexT> void Interleave_Generic(VertexT *dst, const typename VertexT::source_t& src, size_t num);

} // namespace mu
