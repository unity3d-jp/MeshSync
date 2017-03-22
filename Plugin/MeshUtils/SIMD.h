#pragma once

namespace mu {

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
void Lerp(float *dst, const float *src1, const float *src2, size_t num, float w);
void Lerp(float2 *dst, const float2 *src1, const float2 *src2, size_t num, float w);
void Lerp(float3 *dst, const float3 *src1, const float3 *src2, size_t num, float w);
float3 Min(const float3 *src, size_t num);
float3 Max(const float3 *src, size_t num);
void MinMax(const float3 *src, size_t num, float3& dst_min, float3& dst_max);
bool NearEqual(const float *src1, const float *src2, size_t num, float eps);
bool NearEqual(const float2 *src1, const float2 *src2, size_t num, float eps);
bool NearEqual(const float3 *src1, const float3 *src2, size_t num, float eps);

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

void Lerp_Generic(float *dst, const float *src1, const float *src2, size_t num, float w);
void Lerp_ISPC(float *dst, const float *src1, const float *src2, size_t num, float w);

float3 Min_Generic(const float3 *src, size_t num);
float3 Min_ISPC(const float3 *src, size_t num);

float3 Max_Generic(const float3 *src, size_t num);
float3 Max_ISPC(const float3 *src, size_t num);

void MinMax_Generic(const float3 *src, size_t num, float3& dst_min, float3& dst_max);
void MinMax_ISPC(const float3 *src, size_t num, float3& dst_min, float3& dst_max);

bool NearEqual_Generic(const float *src1, const float *src, size_t num, float eps);
bool NearEqual_ISPC(const float *src1, const float *src, size_t num, float eps);

} // namespace mu
