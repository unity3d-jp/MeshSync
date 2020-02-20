#pragma once

#include <vector>
#include <memory>
#include "muRawVector.h"
#include "muIntrusiveArray.h"
#include "muHalf.h"
#include "muMath.h"
#include "muQuat32.h"
#include "muLimits.h"
#include "muSIMD.h"
#include "muAlgorithm.h"
#include "muVertex.h"
#include "muColor.h"
#include "muTLS.h"
#include "muMisc.h"
#include "muConcurrency.h"
#include "muCompression.h"
#include "muStream.h"
#include "muDebugTimer.h"

namespace mu {

struct MeshConnectionInfo;

bool GenerateNormalsPoly(RawVector<float3>& dst,
    const IArray<float3> points, const IArray<int> counts, const IArray<int> indices, bool flip);

void GenerateNormalsWithSmoothAngle(RawVector<float3>& dst,
    const IArray<float3> points,
    const IArray<int> counts, const IArray<int> indices,
    float smooth_angle, bool flip);


// PointsIter: indexed_iterator<const float3*, int*> or indexed_iterator_s<const float3*, int*>
template<class PointsIter>
void GenerateNormalsPoly(float3 *dst,
    PointsIter vertices, const int *counts, const int *offsets, const int *indices,
    int num_faces, int num_vertices);

// PointsIter: indexed_iterator<const float3*, int*> or indexed_iterator_s<const float3*, int*>
// UVIter: indexed_iterator<const float2*, int*> or indexed_iterator_s<const float2*, int*>
template<class PointsIter, class UVIter>
void GenerateTangentsPoly(float4 *dst,
    PointsIter vertices, UVIter uv, const float3 *normals,
    const int *counts, const int *offsets, const int *indices,
    int num_faces, int num_vertices);

void QuadifyTriangles(const IArray<float3> vertices, const IArray<int> triangle_indices, bool full_search, float threshold_angle,
    RawVector<int>& dst_indices, RawVector<int>& dst_counts);

template<class Handler>
void SelectEdge(const IArray<int>& indices, int ngon, const IArray<float3>& vertices,
    const IArray<int>& vertex_indices, const Handler& handler);
template<class Handler>
void SelectEdge(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices,
    const IArray<int>& vertex_indices, const Handler& handler);

template<class Handler>
void SelectHole(const IArray<int>& indices, int ngon, const IArray<float3>& vertices,
    const IArray<int>& vertex_indices, const Handler& handler);
template<class Handler>
void SelectHole(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices,
    const IArray<int>& vertex_indices, const Handler& handler);

template<class Handler>
void SelectConnected(const IArray<int>& indices, int ngon, const IArray<float3>& vertices,
    const IArray<int>& vertex_indices, const Handler& handler);
template<class Handler>
void SelectConnected(const IArray<int>& indices, const IArray<int>& counts, const IArray<int>& offsets, const IArray<float3>& vertices,
    const IArray<int>& vertex_indices, const Handler& handler);


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
        for (int ci = 0; ci < count; ++ci)
            body(fi, i + ci);
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
    if (!dst || !src)
        return;

    size_t size = end - beg;
    for (int i = 0; i < (int)size; ++i) {
        dst[i] = src[indices[beg + i]];
    }
}

template<class T>
inline void CopyWithIndices(T *dst, const T *src, const IArray<int> indices)
{
    if (!dst || !src)
        return;

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


inline void MirrorPoints(float3 *dst, size_t n, float3 plane_n, float plane_d)
{
    if (!dst)
        return;
    for (size_t i = 0; i < n; ++i)
        dst[i] = plane_mirror(dst[i], plane_n, plane_d);
}

inline void MirrorVectors(float3 *dst, size_t n, float3 plane_n)
{
    if (!dst)
        return;
    for (size_t i = 0; i < n; ++i)
        dst[i] = plane_mirror(dst[i], plane_n);
}
inline void MirrorVectors(float4 *dst, size_t n, float3 plane_n)
{
    if (!dst)
        return;
    for (size_t i = 0; i < n; ++i)
        (float3&)dst[i] = plane_mirror((float3&)dst[i], plane_n);
}

inline void MirrorTopology(int *dst_counts, int *dst_indices, const IArray<int>& counts, const IArray<int>& indices, int offset)
{
    if (!dst_counts || !dst_indices)
        return;

    memcpy(dst_counts, counts.data(), sizeof(int) * counts.size());
    EnumerateReverseFaceIndices(counts, [&](int, int idx, int ridx) {
        dst_indices[idx] = offset + indices[ridx];
    });
}

inline void MirrorTopology(int *dst_counts, int *dst_indices, const IArray<int>& counts, const IArray<int>& indices, const IArray<int>& indirect)
{
    if (!dst_counts || !dst_indices)
        return;

    memcpy(dst_counts, counts.data(), sizeof(int) * counts.size());
    EnumerateReverseFaceIndices(counts, [&](int, int idx, int ridx) {
        dst_indices[idx] = indirect[indices[ridx]];
    });
}

} // namespace mu

#include "MeshUtils_impl.h"
#include "muMeshRefiner.h"
