#include "pch.h"
#include "muMath.h"
#include "muSIMD.h"
#include "muRawVector.h"

namespace mu {


#ifdef muEnableISPC
#include "MeshUtilsCore.h"

#ifdef muSIMD_SumInt32
uint64_t SumInt32_ISPC(const uint32_t *src, size_t num)
{
    return ispc::SumInt32(src, (int)num);
}
#endif

#ifdef muSIMD_Float_Half_Conversion
void F32ToF16_ISPC(half *dst, const float *src, size_t num) { ispc::F32ToF16((uint16_t*)dst, src, (int)num); }
void F16ToF32_ISPC(float *dst, const half *src, size_t num) { ispc::F16ToF32(dst, (const uint16_t*)src, (int)num); }
#endif

#ifdef muSIMD_Float_Norm_Conversion
void F32ToS8_ISPC(snorm8 *dst, const float *src, size_t num) { ispc::F32ToS8((int8_t*)dst, src, (int)num); }
void S8ToF32_ISPC(float *dst, const snorm8 *src, size_t num) { ispc::S8ToF32(dst, (int8_t*)src, (int)num); }
void F32ToU8_ISPC(unorm8 *dst, const float *src, size_t num) { ispc::F32ToU8((uint8_t*)dst, src, (int)num); }
void U8ToF32_ISPC(float *dst, const unorm8 *src, size_t num) { ispc::U8ToF32(dst, (uint8_t*)src, (int)num); }
void F32ToU8N_ISPC(unorm8n *dst, const float *src, size_t num) { ispc::F32ToU8N((uint8_t*)dst, src, (int)num); }
void U8NToF32_ISPC(float *dst, const unorm8n *src, size_t num) { ispc::U8NToF32(dst, (uint8_t*)src, (int)num); }
void F32ToS16_ISPC(snorm16 *dst, const float *src, size_t num) { ispc::F32ToS16((int16_t*)dst, src, (int)num); }
void S16ToF32_ISPC(float *dst, const snorm16 *src, size_t num) { ispc::S16ToF32(dst, (int16_t*)src, (int)num); }
void F32ToU16_ISPC(unorm16 *dst, const float *src, size_t num) { ispc::F32ToU16((uint16_t*)dst, src, (int)num); }
void U16ToF32_ISPC(float *dst, const unorm16 *src, size_t num) { ispc::U16ToF32(dst, (uint16_t*)src, (int)num); }
void F32ToS24_ISPC(snorm24 *dst, const float *src, size_t num) { ispc::F32ToS24((uint8_t*)dst, src, (int)num); }
void S24ToF32_ISPC(float *dst, const snorm24 *src, size_t num) { ispc::S24ToF32(dst, (uint8_t*)src, (int)num); }
void F32ToS32_ISPC(snorm32 *dst, const float *src, size_t num) { ispc::F32ToS32((int32_t*)dst, src, (int)num); }
void S32ToF32_ISPC(float *dst, const snorm32 *src, size_t num) { ispc::S32ToF32(dst, (int32_t*)src, (int)num); }
#endif


#ifdef muSIMD_InvertX3
void InvertX_ISPC(float3 *dst, size_t num)
{
    ispc::InvertX3((ispc::float3*)dst, (int)num);
}
#endif
#ifdef muSIMD_InvertX4
void InvertX_ISPC(float4 *dst, size_t num)
{
    ispc::InvertX4((ispc::float4*)dst, (int)num);
}
#endif

#ifdef muSIMD_Scale
void Scale_ISPC(float *dst, float s, size_t num)
{
    ispc::Scale((float*)dst, s, (int)num * 1);
}
#endif
#ifdef muSIMD_Scale
void Scale_ISPC(float3 *dst, float s, size_t num)
{
    ispc::Scale((float*)dst, s, (int)num * 3);
}
#endif

#ifdef muSIMD_Normalize
void Normalize_ISPC(float3 *dst, size_t num)
{
    ispc::Normalize((ispc::float3*)dst, (int)num);
}
#endif

#ifdef muSIMD_Lerp
void Lerp_ISPC(float *dst, const float *src1, const float *src2, size_t num, float w)
{
    ispc::Lerp(dst, src1, src2, (int)num, w);
}
#endif

#ifdef muSIMD_NearEqual
bool NearEqual_ISPC(const float *src1, const float *src2, size_t num, float eps)
{
    return ispc::NearEqual(src1, src2, (int)num, eps);
}
#endif

#ifdef muSIMD_MinMax
void MinMax_ISPC(const int *src, size_t num, int& dst_min, int& dst_max)
{
    ispc::MinMax1I(src, (int)num, dst_min, dst_max);
}
void MinMax_ISPC(const float *src, size_t num, float& dst_min, float& dst_max)
{
    ispc::MinMax1(src, (int)num, dst_min, dst_max);
}
void MinMax_ISPC(const float2 *src, size_t num, float2& dst_min, float2& dst_max)
{
    ispc::MinMax2((ispc::float2*)src, (int)num, (ispc::float2&)dst_min, (ispc::float2&)dst_max);
}
void MinMax_ISPC(const float3 *src, size_t num, float3& dst_min, float3& dst_max)
{
    ispc::MinMax3((ispc::float3*)src, (int)num, (ispc::float3&)dst_min, (ispc::float3&)dst_max);
}
void MinMax_ISPC(const float4 *src, size_t num, float4& dst_min, float4& dst_max)
{
    ispc::MinMax4((ispc::float4*)src, (int)num, (ispc::float4&)dst_min, (ispc::float4&)dst_max);
}
#endif

#ifdef muSIMD_MulPoints3
void MulPoints_ISPC(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    ispc::MulPoints3((ispc::float4x4&)m, (ispc::float3*)src, (ispc::float3*)dst, (int)num_data);
}
#endif
#ifdef muSIMD_MulVectors3
void MulVectors_ISPC(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    ispc::MulVectors3((ispc::float4x4&)m, (ispc::float3*)src, (ispc::float3*)dst, (int)num_data);
}
#endif


#ifdef muSIMD_RayTrianglesIntersectionIndexed
int RayTrianglesIntersectionIndexed_ISPC(
    float3 pos, float3 dir, const float3 *vertices, const int *indices, int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionIndexed(
        (ispc::float3&)pos, (ispc::float3&)dir, (ispc::float3*)vertices, indices, num_triangles, tindex, distance);
}
#endif

#ifdef muSIMD_RayTrianglesIntersectionFlattened
int RayTrianglesIntersectionFlattened_ISPC(
    float3 pos, float3 dir, const float3 *vertices, int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionFlattened(
        (ispc::float3&)pos, (ispc::float3&)dir, (ispc::float3*)vertices, num_triangles, tindex, distance);
}
#endif

#ifdef muSIMD_RayTrianglesIntersectionSoA
int RayTrianglesIntersectionSoA_ISPC(float3 pos, float3 dir,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    int num_triangles, int& tindex, float& distance)
{
    return ispc::RayTrianglesIntersectionSoA(
        (ispc::float3&)pos, (ispc::float3&)dir, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, num_triangles, tindex, distance);
}
#endif


#ifdef muSIMD_PolyInside
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

#ifdef muSIMD_PolyInsideSoA
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
#endif

#ifdef muSIMD_GenerateNormalsTriangleIndexed
void GenerateNormalsTriangleIndexed_ISPC(float3 *dst,
    const float3 *vertices, const int *indices, int num_triangles, int num_vertices)
{
    ispc::GenerateNormalsTriangleIndexed((ispc::float3*)dst, (ispc::float3*)vertices, indices, num_triangles, num_vertices);
}
#endif

#ifdef muSIMD_GenerateNormalsTriangleFlattened
void GenerateNormalsTriangleFlattened_ISPC(float3 *dst,
    const float3 *vertices, const int *indices, int num_triangles, int num_vertices)
{
    ispc::GenerateNormalsTriangleFlattened((ispc::float3*)dst, (ispc::float3*)vertices, indices, num_triangles, num_vertices);
}
#endif

#ifdef muSIMD_GenerateNormalsTriangleSoA
void GenerateNormalsTriangleSoA_ISPC(float3 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const int *indices, int num_triangles, int num_vertices)
{
    ispc::GenerateNormalsTriangleSoA((ispc::float3*)dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        indices, num_triangles, num_vertices);
}
#endif

#ifdef muSIMD_GenerateTangentsTriangleIndexed
void GenerateTangentsTriangleIndexed_ISPC(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices, int num_triangles, int num_vertices)
{
    ispc::GenerateTangentsTriangleIndexed((ispc::float4*)dst,
        (ispc::float3*)vertices, (ispc::float2*)uv, (ispc::float3*)normals, indices,
        num_triangles, num_vertices);
}
#endif

#ifdef muSIMD_GenerateTangentsTriangleFlattened
void GenerateTangentsTriangleFlattened_ISPC(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices, int num_triangles, int num_vertices)
{
    ispc::GenerateTangentsTriangleFlattened((ispc::float4*)dst,
        (ispc::float3*)vertices, (ispc::float2*)uv, (ispc::float3*)normals, indices,
        num_triangles, num_vertices);
}
#endif

#ifdef muSIMD_GenerateTangentsTriangleSoA
void GenerateTangentsTriangleSoA_ISPC(float4 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const float *u1x, const float *u1y,
    const float *u2x, const float *u2y,
    const float *u3x, const float *u3y,
    const float3 *normals,
    const int *indices, int num_triangles, int num_vertices)
{
    ispc::GenerateTangentsTriangleSoA(
        (ispc::float4*)dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        u1x, u1y, u2x, u2y, u3x, u3y,
        (ispc::float3*)normals,
        indices,
        num_triangles, num_vertices);
}
#endif
#endif // muEnableISPC


#ifdef muEnableISPC
    #define Forward(Name, ...) Name##_ISPC(__VA_ARGS__)
#else
    #define Forward(Name, ...) Name##_Generic(__VA_ARGS__)
#endif

#if defined(muSIMD_SumInt32) || !defined(muEnableISPC)
uint64_t SumInt32(const void *src, size_t num)
{
    return Forward(SumInt32, (uint32_t*)src, num / sizeof(uint32_t));
}
#endif

#if defined(muSIMD_Float_Half_Conversion) || !defined(muEnableISPC)
void F32ToF16(half *dst, const float *src, size_t num) { Forward(F32ToF16, dst, src, num); }
void F16ToF32(float *dst, const half *src, size_t num) { Forward(F16ToF32, dst, src, num); }
#endif

#if defined(muSIMD_Float_Norm_Conversion) || !defined(muEnableISPC)
void F32ToS8(snorm8 *dst, const float *src, size_t num) { Forward(F32ToS8, dst, src, num); }
void S8ToF32(float *dst, const snorm8 *src, size_t num) { Forward(S8ToF32, dst, src, num); }
void F32ToU8(unorm8 *dst, const float *src, size_t num) { Forward(F32ToU8, dst, src, num); }
void U8ToF32(float *dst, const unorm8 *src, size_t num) { Forward(U8ToF32, dst, src, num); }
void F32ToU8N(unorm8n *dst, const float *src, size_t num) { Forward(F32ToU8N, dst, src, num); }
void U8NToF32(float *dst, const unorm8n *src, size_t num) { Forward(U8NToF32, dst, src, num); }
void F32ToS16(snorm16 *dst, const float *src, size_t num) { Forward(F32ToS16, dst, src, num); }
void S16ToF32(float *dst, const snorm16 *src, size_t num) { Forward(S16ToF32, dst, src, num); }
void F32ToU16(unorm16 *dst, const float *src, size_t num) { Forward(F32ToU16, dst, src, num); }
void U16ToF32(float *dst, const unorm16 *src, size_t num) { Forward(U16ToF32, dst, src, num); }
void F32ToS24(snorm24 *dst, const float *src, size_t num) { Forward(F32ToS24, dst, src, num); }
void S24ToF32(float *dst, const snorm24 *src, size_t num) { Forward(S24ToF32, dst, src, num); }
void F32ToS32(snorm32 *dst, const float *src, size_t num) { Forward(F32ToS32, dst, src, num); }
void S32ToF32(float *dst, const snorm32 *src, size_t num) { Forward(S32ToF32, dst, src, num); }
#endif


#if defined(muSIMD_InvertX3) || !defined(muEnableISPC)
void InvertX(float3 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}
#endif
#if defined(muSIMD_InvertX4) || !defined(muEnableISPC)
void InvertX(float4 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}
#endif

#if defined(muSIMD_Scale) || !defined(muEnableISPC)
void Scale(float *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}
#endif
#if defined(muSIMD_Scale) || !defined(muEnableISPC)
void Scale(float3 *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}
#endif

#if defined(muSIMD_Normalize) || !defined(muEnableISPC)
void Normalize(float3 *dst, size_t num)
{
    Forward(Normalize, dst, num);
}
#endif

#if defined(muSIMD_Lerp) || !defined(muEnableISPC)
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
void Lerp(float4 *dst, const float4 *src1, const float4 *src2, size_t num, float w)
{
    Lerp((float*)dst, (const float*)src1, (const float*)src2, num * 4, w);
}
#endif

#if defined(muSIMD_MinMax) || !defined(muEnableISPC)
void MinMax(const int *p, size_t num, int& dst_min, int& dst_max) { Forward(MinMax, p, num, dst_min, dst_max); }
void MinMax(const float *p, size_t num, float& dst_min, float& dst_max) { Forward(MinMax, p, num, dst_min, dst_max); }
void MinMax(const float2 *p, size_t num, float2& dst_min, float2& dst_max) { Forward(MinMax, p, num, dst_min, dst_max); }
void MinMax(const float3 *p, size_t num, float3& dst_min, float3& dst_max) { Forward(MinMax, p, num, dst_min, dst_max); }
void MinMax(const float4 *p, size_t num, float4& dst_min, float4& dst_max) { Forward(MinMax, p, num, dst_min, dst_max); }
#endif

#if defined(muSIMD_NearEqual) || !defined(muEnableISPC)
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
bool NearEqual(const float4 *src1, const float4 *src2, size_t num, float eps)
{
    return NearEqual((const float*)src1, (const float*)src2, num * 4, eps);
}
#endif

#if defined(muSIMD_MulPoints3) || !defined(muEnableISPC)
void MulPoints(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    Forward(MulPoints, m, src, dst, num_data);
}
#endif
#if defined(muSIMD_MulVectors3) || !defined(muEnableISPC)
void MulVectors(const float4x4& m, const float3 src[], float3 dst[], size_t num_data)
{
    Forward(MulVectors, m, src, dst, num_data);
}
#endif

#if defined(muSIMD_RayTrianglesIntersectionIndexed) || !defined(muEnableISPC)
int RayTrianglesIntersectionIndexed(float3 pos, float3 dir, const float3 *vertices, const int *indices, int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersectionIndexed, pos, dir, vertices, indices, num_triangles, tindex, result);
}
#endif
#if defined(muSIMD_RayTrianglesIntersectionFlattened) || !defined(muEnableISPC)
int RayTrianglesIntersectionFlattened(float3 pos, float3 dir, const float3 *vertices, int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersectionFlattened, pos, dir, vertices, num_triangles, tindex, result);
}
#endif
#if defined(muSIMD_RayTrianglesIntersectionSoA) || !defined(muEnableISPC)
int RayTrianglesIntersectionSoA(float3 pos, float3 dir,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    int num_triangles, int& tindex, float& result)
{
    return Forward(RayTrianglesIntersectionSoA, pos, dir, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, num_triangles, tindex, result);
}
#endif

#if defined(muSIMD_PolyInside) || !defined(muEnableISPC)
bool PolyInside(const float2 poly[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return Forward(PolyInside, poly, ngon, minp, maxp, pos);
}
#endif
#if defined(muSIMD_PolyInside) || !defined(muEnableISPC)
bool PolyInside(const float2 poly[], int ngon, const float2 pos)
{
    return Forward(PolyInside, poly, ngon, pos);
}
#endif
#if defined(muSIMD_PolyInsideSoA) || !defined(muEnableISPC)
bool PolyInside(const float px[], const float py[], int ngon, const float2 minp, const float2 maxp, const float2 pos)
{
    return Forward(PolyInside, px, py, ngon, minp, maxp, pos);
}
#endif

#if defined(muSIMD_GenerateNormalsTriangleIndexed) || !defined(muEnableISPC)
void GenerateNormalsTriangleIndexed(float3 *dst,
    const float3 *vertices, const int *indices, int num_triangles, int num_vertices)
{
    return Forward(GenerateNormalsTriangleIndexed, dst, vertices, indices, num_triangles, num_vertices);
}
#endif
#if defined(muSIMD_GenerateNormalsTriangleFlattened) || !defined(muEnableISPC)
void GenerateNormalsTriangleFlattened(float3 *dst,
    const float3 *vertices, const int *indices,
    int num_triangles, int num_vertices)
{
    return Forward(GenerateNormalsTriangleFlattened, dst, vertices, indices, num_triangles, num_vertices);
}
#endif
#if defined(muSIMD_GenerateNormalsTriangleSoA) || !defined(muEnableISPC)
void GenerateNormalsTriangleSoA(float3 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const int *indices, int num_triangles, int num_vertices)
{
    return Forward(GenerateNormalsTriangleSoA, dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        indices, num_triangles, num_vertices);
}
#endif


#if defined(muSIMD_GenerateTangentsTriangleIndexed) || !defined(muEnableISPC)
void GenerateTangentsTriangleIndexed(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices,
    int num_triangles, int num_vertices)
{
    return Forward(GenerateTangentsTriangleIndexed, dst, vertices, uv, normals, indices, num_triangles, num_vertices);
}
#endif
#if defined(muSIMD_GenerateTangentsTriangleFlattened) || !defined(muEnableISPC)
void GenerateTangentsTriangleFlattened(float4 *dst,
    const float3 *vertices, const float2 *uv, const float3 *normals, const int *indices,
    int num_triangles, int num_vertices)
{
    return Forward(GenerateTangentsTriangleFlattened, dst, vertices, uv, normals, indices, num_triangles, num_vertices);
}
#endif
#if defined(muSIMD_GenerateTangentsTriangleSoA) || !defined(muEnableISPC)
void GenerateTangentsTriangleSoA(float4 *dst,
    const float *v1x, const float *v1y, const float *v1z,
    const float *v2x, const float *v2y, const float *v2z,
    const float *v3x, const float *v3y, const float *v3z,
    const float *u1x, const float *u1y,
    const float *u2x, const float *u2y,
    const float *u3x, const float *u3y,
    const float3 *normals,
    const int *indices, int num_triangles, int num_vertices)
{
    return Forward(GenerateTangentsTriangleSoA, dst,
        v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
        u1x, u1y, u2x, u2y, u3x, u3y,
        normals, indices, num_triangles, num_vertices);
}
#endif

#undef Forward
} // namespace mu
