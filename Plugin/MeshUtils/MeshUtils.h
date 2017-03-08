#pragma once

#include <vector>
#include <memory>
#include "Math.h"
#include "RawVector.h"
#include "IntrusiveArray.h"

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

// size of dst must be num_points
bool GenerateNormals(
    IArray<float3> dst, const IArray<float3> points,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices);

bool GenerateTangents(
    IArray<float4> dst, const IArray<float3> points, const IArray<float3> normals, const IArray<float2> uv,
    const IArray<int> counts, const IArray<int> offsets, const IArray<int> indices);

template<int N>
bool GenerateWeightsN(RawVector<Weights<N>>& dst, IArray<int> bone_indices, IArray<float> bone_weights, int bones_per_vertex);


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

template<class DstArray, class SrcArray>
void CopyWithIndices(DstArray& dst, const SrcArray& src, const SrcArray& indices, size_t beg, size_t end);



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

struct MeshRefiner
{
    struct Submesh
    {
        int num_indices_tri = 0;
        int materialID = 0;
        int* faces_to_write = nullptr;
    };

    struct Split
    {
        int num_faces = 0;
        int num_vertices = 0;
        int num_indices = 0;
        int num_indices_triangulated = 0;
        int num_submeshes = 0;
    };


    int split_unit = 0; // 0 == no split
    bool triangulate = true;
    bool swap_faces = false;

    IArray<int> counts;
    IArray<int> indices;
    IArray<float3> points;
    IArray<float3> normals;
    IArray<float2> uv;
    IArray<float4> colors;
    IArray<Weights4> weights4;
    IArray<float3> npoints; // points for normal calculation

    RawVector<Submesh> submeshes;
    RawVector<Split> splits;

private:
    RawVector<int> counts_tmp;
    RawVector<int> offsets;
    RawVector<int> v2f_counts;
    RawVector<int> v2f_offsets;
    RawVector<int> shared_faces;
    RawVector<int> shared_indices;
    RawVector<float3> face_normals;
    RawVector<float3> normals_tmp;
    RawVector<float4> tangents_tmp;

    RawVector<float3> new_points;
    RawVector<float3> new_normals;
    RawVector<float4> new_tangents;
    RawVector<float2> new_uv;
    RawVector<float4> new_colors;
    RawVector<Weights4> new_weights4;
    RawVector<int>    new_indices;
    RawVector<int>    new_indices_triangulated;
    RawVector<int>    new_indices_submeshes;
    RawVector<int>    old2new;
    int num_indices_tri = 0;

public:
    void prepare(const IArray<int>& counts, const IArray<int>& indices, const IArray<float3>& points);
    void genNormals();
    void genNormals(float smooth_angle);
    void genTangents();

    bool refine(bool optimize);

    // should be called after refine(), and only valid for triangulated meshes
    bool genSubmesh(const IArray<int>& materialIDs);

    void swapNewData(
        RawVector<float3>& p,
        RawVector<float3>& n,
        RawVector<float4>& t,
        RawVector<float2>& u,
        RawVector<float4>& c,
        RawVector<Weights4>& w,
        RawVector<int>& idx);

private:
    bool refineDumb();
    bool refineWithOptimization();
    void buildConnection();

    template<class Body> void doRefine(const Body& body);
    int findOrAddVertexPNTUC(int vi, const float3& p, const float3& n, const float4& t, const float2& u, const float4& c);
    int findOrAddVertexPNTU(int vi, const float3& p, const float3& n, const float4& t, const float2& u);
    int findOrAddVertexPNU(int vi, const float3& p, const float3& n, const float2& u);
    int findOrAddVertexPN(int vi, const float3& p, const float3& n);
    int findOrAddVertexPU(int vi, const float3& p, const float2& u);
};

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

} // namespace mu
