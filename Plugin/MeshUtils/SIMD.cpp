#include "pch.h"
#include "MeshUtils.h"
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

void ComputeBounds_Generic(const float3 *p, size_t num, float3& omin, float3& omax)
{
    if (num == 0) { return; }
    float3 rmin = p[0], rmax = p[0];
    for (size_t i = 1; i < num; ++i) {
        auto _ = p[i];
        rmin.x = std::min<float>(rmin.x, _.x);
        rmin.y = std::min<float>(rmin.y, _.y);
        rmin.z = std::min<float>(rmin.z, _.z);
        rmax.x = std::max<float>(rmax.x, _.x);
        rmax.y = std::max<float>(rmax.y, _.y);
        rmax.z = std::max<float>(rmax.z, _.z);
    }
    omin = rmin;
    omax = rmax;
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

float3 Min_Generic(const float3 *src, size_t num)
{
    float3 ret = src[0];
    for (size_t i = 1; i < num; ++i) {
        ret[0] = std::min<float>(ret[0], src[i][0]);
        ret[1] = std::min<float>(ret[1], src[i][1]);
        ret[2] = std::min<float>(ret[2], src[i][2]);
    }
    return ret;
}

float3 Max_Generic(const float3 *src, size_t num)
{
    float3 ret = src[0];
    for (size_t i = 1; i < num; ++i) {
        ret[0] = std::max<float>(ret[0], src[i][0]);
        ret[1] = std::max<float>(ret[1], src[i][1]);
        ret[2] = std::max<float>(ret[2], src[i][2]);
    }
    return ret;
}

void MinMax_Generic(const float3 *src, size_t num, float3& dst_min, float3& dst_max)
{
    if (num == 0) { return; }
    float3 rmin = src[0];
    float3 rmax = src[0];
    for (size_t i = 1; i < num; ++i) {
        rmin[0] = std::min<float>(rmin[0], src[i][0]);
        rmin[1] = std::min<float>(rmin[1], src[i][1]);
        rmin[2] = std::min<float>(rmin[2], src[i][2]);

        rmax[0] = std::max<float>(rmax[0], src[i][0]);
        rmax[1] = std::max<float>(rmax[1], src[i][1]);
        rmax[2] = std::max<float>(rmax[2], src[i][2]);
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


#ifdef muEnableISPC
#include "MeshUtilsCore.h"

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

void ComputeBounds_ISPC(const float3 *p, size_t num, float3& omin, float3& omax)
{
    if (num == 0) { return; }
    ispc::ComputeBounds((ispc::float3*)p, (int)num, (ispc::float3&)omin, (ispc::float3&)omax);
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
float3 Min_ISPC(const float3 *src, size_t num)
{
    auto ret = ispc::Min((ispc::float3*)src, (int)num);
    return (const float3&)ret;
}
float3 Max_ISPC(const float3 *src, size_t num)
{
    auto ret = ispc::Max((ispc::float3*)src, (int)num);
    return (const float3&)ret;
}
void MinMax_ISPC(const float3 *src, size_t num, float3& dst_min, float3& dst_max)
{
    if (num == 0) { return; }
    ispc::MinMax((ispc::float3*)src, (int)num, (ispc::float3&)dst_min, (ispc::float3&)dst_max);
}
bool NearEqual_ISPC(const float *src1, const float *src2, size_t num, float eps)
{
    return ispc::NearEqual(src1, src2, (int)num, eps);
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

void ComputeBounds(const float3 *p, size_t num, float3& omin, float3& omax)
{
    Forward(ComputeBounds, p, num, omin, omax);
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

float3 Min(const float3 *p, size_t num)
{
    return Forward(Min, p, num);
}

float3 Max(const float3 *p, size_t num)
{
    return Forward(Max, p, num);
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

#undef Forward

} // namespace mu
