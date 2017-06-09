#include "ispcmath.h"

export void FloatToHalf(
    uniform half dst[],
    uniform const float src[],
    uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] = float_to_half(src[i]);
    }
}

export void HalfToFloat(
    uniform float dst[],
    uniform const half src[],
    uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] = half_to_float(src[i]);
    }
}


// invert every x elements of float3 array
export void InvertXF3(uniform float3 dst[], uniform const int num)
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

// invert every x elements of float4 array
export void InvertXF4(uniform float4 dst[], uniform const int num)
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


// apply scale to every elements
export void ScaleF(uniform float dst[], uniform const float scale, uniform const int num)
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

export void Normalize(
    uniform float3 dst[],
    uniform const int num)
{
    uniform const int num_loops = num / C;

    uniform float * uniform fv = (uniform float * uniform)dst;
    for(uniform int i=0; i < num_loops; ++i) {
        uniform const int i3 = i*3;
        float x,y,z;
        aos_to_soa3((uniform float*)&fv[C*i3], &x, &y, &z);
        float d = rsqrt(x*x + y*y + z*z);
        x *= d;
        y *= d;
        z *= d;
        soa_to_aos3(x, y, z, (uniform float*)&fv[C*i3]);
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        uniform float3 t = dst[i];
        uniform float d = rsqrt(t.x*t.x + t.y*t.y + t.z*t.z);
        t.x *= d;
        t.y *= d;
        t.z *= d;
        dst[i] = t;
    }
}


export void Lerp(uniform float dst[], uniform const float src1[], uniform const float src2[], uniform const int num, uniform float w)
{
    uniform float iw = 1.0f - w;
    foreach(i=0 ... num) {
        dst[i] = src1[i]*w + src2[i]*iw;
    }
}

export uniform bool NearEqual(
    uniform const float src1[], uniform const float src2[], uniform const int num, uniform const float eps)
{
    float tdiff = 0.0f;
    foreach(i=0 ... num) {
        tdiff = max(tdiff, abs(src1[i] - src2[i]));
    }
    return reduce_max(tdiff) < eps;
}






export uniform int RayTrianglesIntersectionIndexed(
    uniform const float3& pos, uniform const float3& dir,
    uniform const float3 vertices[], uniform const int indices[], uniform const int num_triangles,
    uniform int& tindex, uniform float& distance)
{
    uniform int total_hit = 0;
    distance = FLT_MAX;

    // SIMD pass
    uniform int num_triangles_simd = num_triangles & ~(C - 1);
    for(uniform int bi=0; bi < num_triangles_simd; bi += C) {
        float3 p1, p2, p3;
        // this emits warnings but performace is acceptable.
        p1 = vertices[indices[(bi + I) * 3  + 0]];
        p2 = vertices[indices[(bi + I) * 3  + 1]];
        p3 = vertices[indices[(bi + I) * 3  + 2]];

        float d;
        bool hit = ray_triangle_intersection(pos, dir, p1, p2, p3, d);
        if(any(hit)) {
            uniform int hita[C]; hita[I] = hit;
            uniform float da[C]; da[I] = d;
            for(uniform int i = 0; i < C; ++i) {
                if(hita[i]) {
                    total_hit++;
                    if(da[i] < distance) {
                        tindex = bi + i;
                        distance = da[i];
                    }
                }
            }
        }
    }

    // non-SIMD pass
    for(uniform int ti = num_triangles_simd; ti < num_triangles; ++ti) {
        uniform float3 p1 = vertices[indices[ti * 3 + 0]];
        uniform float3 p2 = vertices[indices[ti * 3 + 1]];
        uniform float3 p3 = vertices[indices[ti * 3 + 2]];

        uniform float d;
        uniform bool hit = ray_triangle_intersection(pos, dir, p1, p2, p3, d);
        if(hit) {
            total_hit++;
            if(d < distance) {
                tindex = ti;
                distance = d;
            }
        }
    }

    return total_hit;
}

export uniform int RayTrianglesIntersectionSoA(
    uniform const float3& pos, uniform const float3& dir,
    uniform const float v1x[], uniform const float v1y[], uniform const float v1z[],
    uniform const float v2x[], uniform const float v2y[], uniform const float v2z[],
    uniform const float v3x[], uniform const float v3y[], uniform const float v3z[],
    uniform const int num_triangles,
    uniform int& tindex, uniform float& distance)
{
    uniform int total_hit = 0;
    distance = FLT_MAX;

    // SIMD pass
    uniform int num_triangles_simd = num_triangles & ~(C - 1);
    for(uniform int bi=0; bi < num_triangles_simd; bi += C) {
        float3 p1 = {v1x[bi+I], v1y[bi+I], v1z[bi+I]};
        float3 p2 = {v2x[bi+I], v2y[bi+I], v2z[bi+I]};
        float3 p3 = {v3x[bi+I], v3y[bi+I], v3z[bi+I]};

        float d;
        bool hit = ray_triangle_intersection(pos, dir, p1, p2, p3, d);
        if(any(hit)) {
            uniform int hita[C]; hita[I] = hit;
            uniform float da[C]; da[I] = d;
            for(uniform int i = 0; i < C; ++i) {
                if(hita[i]) {
                    total_hit++;
                    if(da[i] < distance) {
                        tindex = bi + i;
                        distance = da[i];
                    }
                }
            }
        }
    }

    // non-SIMD pass
    for(uniform int ti = num_triangles_simd; ti < num_triangles; ++ti) {
        uniform float3 p1 = {v1x[ti], v1y[ti], v1z[ti]};
        uniform float3 p2 = {v2x[ti], v2y[ti], v2z[ti]};
        uniform float3 p3 = {v3x[ti], v3y[ti], v3z[ti]};

        uniform float d;
        uniform bool hit = ray_triangle_intersection(pos, dir, p1, p2, p3, d);
        if(hit) {
            total_hit++;
            if(d < distance) {
                tindex = ti;
                distance = d;
            }
        }
    }

    return total_hit;
}
