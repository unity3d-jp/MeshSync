#include "ispcmath.h"

#ifdef muSIMD_FloatToHalf
export void FloatToHalf(
    uniform half dst[],
    uniform const float src[],
    uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] = float_to_half(src[i]);
    }
}
#endif

#ifdef muSIMD_HalfToFloat
export void HalfToFloat(
    uniform float dst[],
    uniform const half src[],
    uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] = half_to_float(src[i]);
    }
}
#endif


#ifdef muSIMD_InvertX3
export void InvertX3(uniform float3 dst[], uniform const int num)
{
    const uniform int num_loops = num / C;

    {
        uniform float _c[3][C];
        _c[0][I] = select((C*0 + I)%3==0, -1.0f, 1.0f);
        _c[1][I] = select((C*1 + I)%3==0, -1.0f, 1.0f);
        _c[2][I] = select((C*2 + I)%3==0, -1.0f, 1.0f);

        uniform float * uniform fv = (uniform float * uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            uniform int i3 = i*3;
            fv[C*(i3+0) + I] = fv[C*(i3+0) + I] * _c[0][I];
            fv[C*(i3+1) + I] = fv[C*(i3+1) + I] * _c[1][I];
            fv[C*(i3+2) + I] = fv[C*(i3+2) + I] * _c[2][I];
        }
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}
#endif

#ifdef muSIMD_InvertX4
export void InvertX4(uniform float4 dst[], uniform const int num)
{
    const uniform int num_loops = num / (C/4);

    {
        uniform float _c[C];
        _c[I] = select(I%4==0, -1.0f, 1.0f);

        uniform float * uniform fv = (uniform float * uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            fv[C*i + I] = fv[C*i + I] * _c[I];
        }
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}
#endif


#ifdef muSIMD_Scale
export void Scale(uniform float dst[], uniform const float scale, uniform const int num)
{
    const uniform int num_loops = num / (C*4);

    {
        uniform float * uniform fv = (uniform float * uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            uniform int i4 = i*4;
            fv[C*(i4+0) + I] = fv[C*(i4+0) + I] * scale;
            fv[C*(i4+1) + I] = fv[C*(i4+1) + I] * scale;
            fv[C*(i4+2) + I] = fv[C*(i4+2) + I] * scale;
            fv[C*(i4+3) + I] = fv[C*(i4+3) + I] * scale;
        }
    }

    for(uniform int i=num_loops*(C*4); i < num; ++i) {
        dst[i] *= scale;
    }
}
#endif

#ifdef muSIMD_Normalize
export void Normalize(
    uniform float3 dst[],
    uniform const int num)
{
    uniform int num_simd = num & ~(C - 1);
    for(uniform int bi=0; bi < num_simd; bi+=C) {
        float3 n;
        aos_to_soa3((uniform float*)&dst[bi], &n.x, &n.y, &n.z);
        n = normalize(n);
        soa_to_aos3(n.x, n.y, n.z, (uniform float*)&dst[bi]);
    }

    for(uniform int i=num_simd; i < num; ++i) {
        dst[i] = normalize(dst[i]);
    }
}
#endif


#ifdef muSIMD_Lerp
export void Lerp(uniform float dst[], uniform const float src1[], uniform const float src2[], uniform const int num, uniform float w)
{
    uniform float iw = 1.0f - w;
    foreach(i=0 ... num) {
        dst[i] = src1[i]*w + src2[i]*iw;
    }
}
#endif

#ifdef muSIMD_NearEqual
export uniform bool NearEqual(
    uniform const float src1[], uniform const float src2[], uniform const int num, uniform const float eps)
{
    float tdiff = 0.0f;
    foreach(i=0 ... num) {
        tdiff = max(tdiff, abs(src1[i] - src2[i]));
    }
    return reduce_max(tdiff) < eps;
}
#endif

