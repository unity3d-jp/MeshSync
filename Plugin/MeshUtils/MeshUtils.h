#pragma once

#include "Math.h"
#include "IntrusiveArray.h"

namespace mu {

extern const float PI;
extern const float Deg2Rad;

#ifdef muEnableHalf
void FloatToHalf(half *dst, const float *src, size_t num);
void HalfToFloat(float *dst, const half *src, size_t num);
#endif // muEnableHalf

void InvertX(float3 *dst, size_t num);
void InvertX(float4 *dst, size_t num);
void InvertV(float2 *dst, size_t num);
void Scale(float *dst, float s, size_t num);
void Scale(float3 *dst, float s, size_t num);
void ComputeBounds(const float3 *p, size_t num, float3& o_min, float3& o_max);
void Normalize(float3 *dst, size_t num);

template<class T>
using IA = IntrusiveArray<T>;

// size of dst must be num_points
bool GenerateNormals(
    IA<float3> dst, const IA<float3> points,
    const IA<int> counts, const IA<int> offsets, const IA<int> indices);

// size of dst must be num_indices
bool GenerateNormalsWithThreshold(
    IA<float3> dst, const IA<float3> points,
    const IA<int> counts, const IA<int> offsets, const IA<int> indices, float threshold);

bool GenerateTangents(
    IA<float4> dst, const IA<float3> points, const IA<float3> normals, const IA<float2> uv,
    const IA<int> counts, const IA<int> offsets, const IA<int> indices);



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

template<class VertexT>
void TInterleave(VertexT *dst, const typename VertexT::arrays_t& src, size_t num);

template<class DataArray, class IndexArray>
void CopyWithIndices(DataArray& dst, const DataArray& src, const IndexArray& indices, size_t beg, size_t end);



// ------------------------------------------------------------
// internal
// ------------------------------------------------------------
#ifdef muEnableHalf
void FloatToHalf_Generic(half *dst, const float *src, size_t num);
void FloatToHalf_ISPC(half *dst, const float *src, size_t num);
void HalfToFloat_Generic(float *dst, const half *src, size_t num);
void HalfToFloat_ISPC(float *dst, const half *src, size_t num);
#endif // muEnableHalf

void InvertX_Generic(float3 *dst, size_t num);
void InvertX_ISPC(float3 *dst, size_t num);
void InvertX_Generic(float4 *dst, size_t num);
void InvertX_ISPC(float4 *dst, size_t num);

void Scale_Generic(float *dst, float s, size_t num);
void Scale_Generic(float3 *dst, float s, size_t num);
void Scale_ISPC(float *dst, float s, size_t num);
void Scale_ISPC(float3 *dst, float s, size_t num);

void ComputeBounds_Generic(const float3 *p, size_t num, float3& o_min, float3& o_max);
void ComputeBounds_ISPC(const float3 *p, size_t num, float3& o_min, float3& o_max);

void Normalize_Generic(float3 *dst, size_t num);
void Normalize_ISPC(float3 *dst, size_t num);

template<class VertexT> void Interleave_Generic(VertexT *dst, const typename VertexT::source_t& src, size_t num);


// ------------------------------------------------------------
// impl
// ------------------------------------------------------------

template<class DataArray, class IndexArray>
inline void CopyWithIndices(DataArray& dst, const DataArray& src, const IndexArray& indices, size_t beg, size_t end)
{
    if (src.empty()) { return; }

    size_t size = end - beg;
    dst.resize(size);

    for (int i = 0; i < (int)size; ++i) {
        dst[i] = src[indices[beg + i]];
    }
}

template<class IntArray>
inline void CountIndices(
    const IntArray &counts,
    IntArray& offsets,
    int& num_indices,
    int& num_indices_triangulated)
{
    int reti = 0, rett = 0;
    size_t num_faces = counts.size();
    offsets.resize(num_faces);
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        auto f = counts[fi];
        offsets[fi] = reti;
        reti += f;
        rett += std::max<int>(f - 2, 0) * 3;
    }
    num_indices = reti;
    num_indices_triangulated = rett;
}

template<class DstArray, class SrcArray>
inline int Triangulate(
    DstArray& dst,
    const SrcArray& counts,
    bool swap_face)
{
    const int i1 = swap_face ? 2 : 1;
    const int i2 = swap_face ? 1 : 2;
    size_t num_faces = counts.size();

    int n = 0;
    int i = 0;
    for (size_t fi = 0; fi < num_faces; ++fi) {
        int count = counts[fi];
        for (int ni = 0; ni < count - 2; ++ni) {
            dst[i + 0] = n + 0;
            dst[i + 1] = n + ni + i1;
            dst[i + 2] = n + ni + i2;
            i += 3;
        }
        n += count;
    }
    return i;
}

template<class DstArray, class SrcArray>
inline void TriangulateWithIndices(
    DstArray& dst,
    const SrcArray& counts,
    const SrcArray& indices,
    bool swap_face)
{
    const int i1 = swap_face ? 2 : 1;
    const int i2 = swap_face ? 1 : 2;
    size_t num_faces = counts.size();

    int n = 0;
    int i = 0;
    for (size_t fi = 0; fi < num_faces; ++fi) {
        int count = counts[fi];
        for (int ni = 0; ni < count - 2; ++ni) {
            dst[i + 0] = indices[n + 0];
            dst[i + 1] = indices[n + ni + i1];
            dst[i + 2] = indices[n + ni + i2];
            i += 3;
        }
        n += count;
    }
}

template<class DstArray, class SrcArray, class TmpArray = DstArray>
inline void BuildConnectedFaceList(
    DstArray& dst_counts, DstArray& dst_offsets, DstArray& dst_connections,
    const SrcArray& counts, const SrcArray& indices, size_t num_points)
{
    size_t num_faces = counts.size();
    size_t num_indices = indices.size();

    dst_counts.resize(num_points);
    dst_offsets.resize(num_points);
    dst_connections.resize(num_indices);
    memset(dst_counts.data(), 0, sizeof(int)*num_points);

    {
        const int *idx = indices.data();
        for (auto& c : counts) {
            for (int i = 0; i < c; ++i) {
                dst_counts[idx[i]]++;
            }
            idx += c;
        }
    }

    TmpArray tmp_indices;
    tmp_indices.resize(num_points);
    memset(tmp_indices.data(), 0, sizeof(int)*num_points);

    {
        int offset = 0;
        for (size_t i = 0; i < num_points; ++i) {
            dst_offsets[i] = offset;
            offset += dst_counts[i];
        }
    }
    {
        const int *idx = indices.data();
        for (int fi = 0; fi < (int)num_faces; ++fi) {
            int c = counts[fi];
            for (int i = 0; i < c; ++i) {
                dst_connections[dst_offsets[idx[i]] + tmp_indices[idx[i]]++] = fi;
            }
            idx += c;
        }
    }
}

// SplitMeshHandler: [](int face_begin, int face_count, int vertex_count) -> void
template<class IndexArray, class SplitMeshHandler>
inline bool Split(const IndexArray& counts, int max_vertices, const SplitMeshHandler& handler)
{
    int face_begin = 0;
    for (int nth = 0; ; ++nth) {
        int num_faces = 0;
        int num_vertices = 0;
        int num_indices_triangulated = 0;
        int face_end = (int)counts.size();
        bool last = true;
        for (int fi = face_begin; fi < face_end; ++fi) {
            int count = counts[fi];
            if (num_vertices + count > max_vertices) {
                last = false;
                if (count >= max_vertices) { return false; }
                break;
            }
            else {
                ++num_faces;
                num_vertices += count;
                num_indices_triangulated += (count - 2) * 3;
            }
        }

        handler(face_begin, num_faces, num_vertices, num_indices_triangulated);

        if (last) { break; }
        face_begin += num_faces;
    }
    return true;
}


template<class Points>
inline void MirrorPoints(float3 *dst, const Points& src, float3 plane_n, float plane_d)
{
    if (!dst || !src.data()) { return; }

    auto n = src.size();
    for (size_t i = 0; i < n; ++i) {
        auto& p = src[i];
        float d = dot(plane_n, p) - plane_d;
        dst[i] = p - (plane_n * (d  * 2.0f));
    }
}

template<class IntArray>
inline void MirrorTopology(int *dst_counts, int *dst_indices, const IntArray& counts, const IntArray& indices, int offset)
{
    if (!dst_counts || !dst_indices) { return; }

    memcpy(dst_counts, counts.data(), sizeof(int) * counts.size());
    size_t i = 0;
    for (int count : counts) {
        for (int ci = 0; ci < count; ++ci) {
            dst_indices[i + ci] = offset + indices[i + (count - ci - 1)];
        }
        i += count;
    }
}

} // namespace mu
