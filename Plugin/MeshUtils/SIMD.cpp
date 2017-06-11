#include "pch.h"
#include "MeshUtils.h"
#include "Concurrency.h"
#include "SIMD.h"

namespace mu {

#ifdef muEnableHalf
void FloatToHalf_Generic(half *dst, const float *src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] = src[i];
    }
}
void HalfToFloat_Generic(float *dst, const half *src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] = src[i];
    }
}
#endif // muEnableHalf

void InvertX_Generic(float3 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}
void InvertX_Generic(float4 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}

void InvertV(float2 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].y = 1.0f - dst[i].y;
    }
}


void Scale_Generic(float *dst, float s, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
}
void Scale_Generic(float3 *dst, float s, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
}

void Normalize_Generic(float3 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] = normalize(dst[i]);
    }
}

void Lerp_Generic(float *dst, const float *src1, const float *src2, size_t num, float w)
{
    const float iw = 1.0f - w;
    for (size_t i = 0; i < num; ++i) {
        dst[i] = src1[i] * w + src2[i] * iw;
    }
}

void MinMax_Generic(const float2 *src, size_t num, float2& dst_min, float2& dst_max)
{
    if (num == 0) { return; }
    float2 rmin = src[0];
    float2 rmax = src[0];
    for (size_t i = 1; i < num; ++i) {
        rmin = min(rmin, src[i]);
        rmax = max(rmax, src[i]);
    }
    dst_min = rmin;
    dst_max = rmax;
}

void MinMax_Generic(const float3 *src, size_t num, float3& dst_min, float3& dst_max)
{
    if (num == 0) { return; }
    float3 rmin = src[0];
    float3 rmax = src[0];
    for (size_t i = 1; i < num; ++i) {
        rmin = min(rmin, src[i]);
        rmax = max(rmax, src[i]);
    }
    dst_min = rmin;
    dst_max = rmax;
}

bool NearEqual_Generic(const float *src1, const float *src2, size_t num, float eps)
{
    for (size_t i = 0; i < num; ++i) {
        if (!near_equal(src1[i], src2[i], eps)) {
            return false;
        }
    }
    return true;
}

int RayTrianglesIntersection_Generic(float3 pos, float3 dir, const float3 *vertices, int num_triangles, int& tindex, float& distance)
{
    int num_hits = 0;
    distance = FLT_MAX;

    for (int i = 0; i < num_triangles; ++i) {
        float d;
        if (ray_triangle_intersection(pos, dir, vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2], d)) {
            ++num_hits;
            if (d < distance) {
                distance = d;
                tindex = i;
            }
        }
    }
    return num_hits;
}
int RayTrianglesIntersection_Generic(float3 pos, float3 dir, const float3 *vertices, const int *indices, int num_triangles, int& tindex, float& distance)
{
    int num_hits = 0;
    distance = FLT_MAX;

    for (int i = 0; i < num_triangles; ++i) {
        float d;
        if (ray_triangle_intersection(pos, dir, vertices[indices[i * 3 + 0]], vertices[indices[i * 3 + 1]], vertices[indices[i * 3 + 2]], d)) {
            ++num_hits;
            if (d < distance) {
                distance = d;
                tindex = i;
            }
        }
    }
    return num_hits;
}
int RayTrianglesIntersection_Generic(float3 pos, float3 dir,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    int num_triangles, int& tindex, float& distance)
{
    int num_hits = 0;
    distance = FLT_MAX;

    for (int i = 0; i < num_triangles; ++i) {
        float d;
        if (ray_triangle_intersection(pos, dir,
            { v1x[i], v1y[i], v1z[i] },
            { v2x[i], v2y[i], v2z[i] },
            { v3x[i], v3y[i], v3z[i] }, d))
        {
            ++num_hits;
            if (d < distance) {
                distance = d;
                tindex = i;
            }
        }
    }
    return num_hits;
}


bool PolyInside_Generic(const float px[], const float py[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return poly_inside(px, py, ngon, minp, maxp, pos);
}

bool PolyInside_Generic(const float2 poly[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return poly_inside(poly, ngon, minp, maxp, pos);
}

bool PolyInside_Generic(const float2 poly[], int ngon, const float2 pos)
{
    float2 minp, maxp;
    poly_minmax(poly, ngon, minp, maxp);
    return poly_inside(poly, ngon, minp, maxp, pos);
}



#ifdef muEnableISPC
#include "MeshUtilsCore.h"
#include "MeshUtilsCore2.h"

#ifdef muEnableHalf
void FloatToHalf_ISPC(half *dst, const float *src, size_t num)
{
    ispc::FloatToHalf((uint16_t*)dst, src, (int)num);
}
void HalfToFloat_ISPC(float *dst, const half *src, size_t num)
{
    ispc::HalfToFloat(dst, (const uint16_t*)src, (int)num);
}
#endif // muEnableHalf

void InvertX_ISPC(float3 *dst, size_t num)
{
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
}
void InvertX_ISPC(float4 *dst, size_t num)
{
    ispc::InvertXF4((ispc::float4*)dst, (int)num);
}

void Scale_ISPC(float *dst, float s, size_t num)
{
    ispc::ScaleF((float*)dst, s, (int)num * 1);
}
void Scale_ISPC(float3 *dst, float s, size_t num)
{
    ispc::ScaleF((float*)dst, s, (int)num * 3);
}

void Normalize_ISPC(float3 *dst, size_t num)
{
    ispc::Normalize((ispc::float3*)dst, (int)num);
}

void GenerateNormals_ISPC(
    float3 *dst, const float3 *p,
    const int *counts, const int *offsets, const int *indices, size_t num_points, size_t num_faces)
{
    memset(dst, 0, sizeof(float3)*num_points);

    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        int count = counts[fi];
        const int *face = &indices[offsets[fi]];
        int i0 = face[0];
        int i1 = face[1];
        int i2 = face[2];
        float3 p0 = p[i0];
        float3 p1 = p[i1];
        float3 p2 = p[i2];
        float3 n = cross(p1 - p0, p2 - p0);
        for (int ci = 0; ci < count; ++ci) {
            dst[face[ci]] += n;
        }
    }

    ispc::Normalize((ispc::float3*)dst, (int)num_points);
}

void Lerp_ISPC(float *dst, const float *src1, const float *src2, size_t num, float w)
{
    ispc::Lerp(dst, src1, src2, (int)num, w);
}

void MinMax_ISPC(const float2 *src, size_t num, float2& dst_min, float2& dst_max)
{
    if (num == 0) { return; }
    ispc::MinMax2((ispc::float2*)src, (int)num, (ispc::float2&)dst_min, (ispc::float2&)dst_max);
}
void MinMax_ISPC(const float3 *src, size_t num, float3& dst_min, float3& dst_max)
{
    if (num == 0) { return; }
    ispc::MinMax3((ispc::float3*)src, (int)num, (ispc::float3&)dst_min, (ispc::float3&)dst_max);
}

bool NearEqual_ISPC(const float *src1, const float *src2, size_t num, float eps)
{
    return ispc::NearEqual(src1, src2, (int)num, eps);
}

int RayTrianglesIntersection_ISPC(
    float3 pos, float3 dir, const float3 *vertices, const int *indices, int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionIndexed(
        (ispc::float3&)pos, (ispc::float3&)dir, (ispc::float3*)vertices, indices, num_triangles, tindex, distance);
}
int RayTrianglesIntersection_ISPC(
    float3 pos, float3 dir, const float3 *vertices, int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionArray(
        (ispc::float3&)pos, (ispc::float3&)dir, (ispc::float3*)vertices, num_triangles, tindex, distance);
}
int RayTrianglesIntersection_ISPC(float3 pos, float3 dir,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionSoA(
        (ispc::float3&)pos, (ispc::float3&)dir, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, num_triangles, tindex, distance);
}


bool PolyInside_ISPC(const float px[], const float py[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    const int MaxXC = 64;
    float xc[MaxXC];

    int c = ispc::PolyInsideSoAImpl(px, py, ngon, (ispc::float2&)minp, (ispc::float2&)maxp, (ispc::float2&)pos, xc, MaxXC);
    std::sort(xc, xc + c);
    for (int i = 0; i < c; i += 2) {
        if (pos.x >= xc[i] && pos.x < xc[i + 1]) {
            return true;
        }
    }
    return false;
}

bool PolyInside_ISPC(const float2 poly[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    const int MaxXC = 64;
    float xc[MaxXC];

    int c = ispc::PolyInsideImpl((ispc::float2*)poly, ngon, (ispc::float2&)minp, (ispc::float2&)maxp, (ispc::float2&)pos, xc, MaxXC);
    std::sort(xc, xc + c);
    for (int i = 0; i < c; i += 2) {
        if (pos.x >= xc[i] && pos.x < xc[i + 1]) {
            return true;
        }
    }
    return false;
}

bool PolyInside_ISPC(const float2 poly[], int ngon, const float2 pos)
{
    float2 minp, maxp;
    MinMax(poly, ngon, minp, maxp);
    return PolyInside_ISPC(poly, ngon, minp, maxp, pos);
}

#endif



#ifdef muEnableISPC
    #define Forward(Name, ...) Name##_ISPC(__VA_ARGS__)
#else
    #define Forward(Name, ...) Name##_Generic(__VA_ARGS__)
#endif

#ifdef muEnableHalf
void FloatToHalf(half *dst, const float *src, size_t num)
{
    Forward(FloatToHalf, dst, src, num);
}
void HalfToFloat(float *dst, const half *src, size_t num)
{
    Forward(HalfToFloat, dst, src, num);
}
#endif // muEnableHalf

void InvertX(float3 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}
void InvertX(float4 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}

void Scale(float *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}
void Scale(float3 *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}

void Normalize(float3 *dst, size_t num)
{
    Forward(Normalize, dst, num);
}

void Lerp(float *dst, const float *src1, const float *src2, size_t num, float w)
{
    Forward(Lerp, dst, src1, src2, num, w);
}
void Lerp(float2 *dst, const float2 *src1, const float2 *src2, size_t num, float w)
{
    Lerp((float*)dst, (const float*)src1, (const float*)src2, num * 2, w);
}
void Lerp(float3 *dst, const float3 *src1, const float3 *src2, size_t num, float w)
{
    Lerp((float*)dst, (const float*)src1, (const float*)src2, num * 3, w);
}

void MinMax(const float2 *p, size_t num, float2& dst_min, float2& dst_max)
{
    Forward(MinMax, p, num, dst_min, dst_max);
}
void MinMax(const float3 *p, size_t num, float3& dst_min, float3& dst_max)
{
    Forward(MinMax, p, num, dst_min, dst_max);
}

bool NearEqual(const float *src1, const float *src2, size_t num, float eps)
{
    return Forward(NearEqual, src1, src2, num, eps);
}
bool NearEqual(const float2 *src1, const float2 *src2, size_t num, float eps)
{
    return NearEqual((const float*)src1, (const float*)src2, num * 2, eps);
}
bool NearEqual(const float3 *src1, const float3 *src2, size_t num, float eps)
{
    return NearEqual((const float*)src1, (const float*)src2, num * 3, eps);
}

int RayTrianglesIntersection(float3 pos, float3 dir, const float3 *vertices, const int *indices, int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersection, pos, dir, vertices, indices, num_triangles, tindex, result);
}
int RayTrianglesIntersection(float3 pos, float3 dir, const float3 *vertices, int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersection, pos, dir, vertices, num_triangles, tindex, result);
}
int RayTrianglesIntersection(float3 pos, float3 dir,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersection, pos, dir, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, num_triangles, tindex, result);
}

bool PolyInside(const float px[], const float py[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return Forward(PolyInside, px, py, ngon, minp, maxp, pos);
}
bool PolyInside(const float2 poly[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return Forward(PolyInside, poly, ngon, minp, maxp, pos);
}
bool PolyInside(const float2 poly[], int ngon, const float2 pos)
{
    return Forward(PolyInside, poly, ngon, pos);
}

#undef Forward
} // namespace mu
