#pragma once

#include <vector>
#include <memory>
#include "Math.h"
#include "RawVector.h"
#include "IntrusiveArray.h"
#include "SIMD.h"
#include "Vertex.h"

namespace mu {

extern const float PI;
extern const float Deg2Rad;
extern const float Rad2Deg;

template<int N>
struct Weights
{
    float   weights[N] = {};
    int     indices[N] = {};
};
using Weights4 = Weights<4>;
using Weights8 = Weights<8>;

// size of dst must be num_points
bool GenerateNormals(
    IArray<float3> dst, const IArray<float3> points,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices);

bool GenerateTangents(
    IArray<float4> dst, const IArray<float3> points, const IArray<float3> normals, const IArray<float2> uv,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices);

template<int N>
bool GenerateWeightsN(RawVector<Weights<N>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex);



// ------------------------------------------------------------
// impl
// ------------------------------------------------------------

// Body: [](int face_index, int vertex_index) -> void
template<class Body>
inline void EnumerateFaceIndices(const IArray<int> counts, const Body& body)
{
    int num_faces = (int)counts.size();
    int i = 0;
    for (int fi = 0; fi < num_faces; ++fi) {
        int count = counts[fi];
        for (int ci = 0; ci < count; ++ci) {
            int index = i + ci;
            body(fi, index);
        }
        i += count;
    }
}

// Body: [](int face_index, int vertex_index, int reverse_vertex_index) -> void
template<class Body>
inline void EnumerateReverseFaceIndices(const IArray<int> counts, const Body& body)
{
    int num_faces = (int)counts.size();
    int i = 0;
    for (int fi = 0; fi < num_faces; ++fi) {
        int count = counts[fi];
        for (int ci = 0; ci < count; ++ci) {
            int index = i + ci;
            int rindex = i + (count - ci - 1);
            body(fi, index, rindex);
        }
        i += count;
    }
}

template<class T>
inline void CopyWithIndices(T *dst, const T *src, const IArray<int> indices, size_t beg, size_t end)
{
    if (!dst || !src) { return; }

    size_t size = end - beg;
    for (int i = 0; i < (int)size; ++i) {
        dst[i] = src[indices[beg + i]];
    }
}

template<class T>
inline void CopyWithIndices(T *dst, const T *src, const IArray<int> indices)
{
    if (!dst || !src) { return; }

    size_t size = indices.size();
    for (int i = 0; i < (int)size; ++i) {
        dst[i] = src[indices[i]];
    }
}

template<class IntArray1, class IntArray2>
inline void CountIndices(
    const IntArray1 &counts,
    IntArray2& offsets,
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



template<class IndexArray, class SplitMeshHandler>
inline bool Split(const IndexArray& counts, int max_vertices, const SplitMeshHandler& handler)
{
    int offset_faces = 0;
    for (int nth = 0; ; ++nth) {
        int num_faces = 0;
        int num_vertices = 0;
        int num_indices_triangulated = 0;
        int face_end = (int)counts.size();
        bool last = true;
        for (int fi = offset_faces; fi < face_end; ++fi) {
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

        handler(num_faces, num_vertices, num_indices_triangulated);

        if (last) { break; }
        offset_faces += num_faces;
    }
    return true;
}


inline void MirrorPoints(float3 *dst, const IArray<float3>& src, float3 plane_n, float plane_d)
{
    if (!dst || !src.data()) { return; }

    auto n = src.size();
    for (size_t i = 0; i < n; ++i) {
        auto& p = src[i];
        float d = dot(plane_n, p) - plane_d;
        dst[i] = p - (plane_n * (d  * 2.0f));
    }
}
inline void MirrorPoints(float3 *dst, const IArray<float3>& src, const IArray<int>& index, float3 plane_n, float plane_d)
{
    if (!dst || !src.data()) { return; }

    auto n = index.size();
    for (size_t i = 0; i < n; ++i) {
        auto& p = src[index[i]];
        float d = dot(plane_n, p) - plane_d;
        dst[i] = p - (plane_n * (d  * 2.0f));
    }
}

inline void MirrorTopology(int *dst_counts, int *dst_indices, const IArray<int>& counts, const IArray<int>& indices, int offset)
{
    if (!dst_counts || !dst_indices) { return; }

    memcpy(dst_counts, counts.data(), sizeof(int) * counts.size());
    EnumerateReverseFaceIndices(counts, [&](int, int idx, int ridx) {
        dst_indices[idx] = offset + indices[ridx];
    });
}

inline void MirrorTopology(int *dst_counts, int *dst_indices, const IArray<int>& counts, const IArray<int>& indices, const IArray<int>& indirect)
{
    if (!dst_counts || !dst_indices) { return; }

    memcpy(dst_counts, counts.data(), sizeof(int) * counts.size());
    EnumerateReverseFaceIndices(counts, [&](int, int idx, int ridx) {
        dst_indices[idx] = indirect[indices[ridx]];
    });
}


inline float4 Color32ToFloat4(uint32_t c)
{
    return{
        (float)(c & 0xff) / 255.0f,
        (float)((c & 0xff00) >> 8) / 255.0f,
        (float)((c & 0xff0000) >> 16) / 255.0f,
        (float)(c >> 24) / 255.0f,
    };
}
inline uint32_t Float4ToColor32(const float4& c)
{
    return
        (int)(c.x * 255.0f) |
        ((int)(c.y * 255.0f) << 8) |
        ((int)(c.z * 255.0f) << 16) |
        ((int)(c.w * 255.0f) << 24);
}

} // namespace mu

#include "MeshRefiner.h"
