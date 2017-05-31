#define C programCount
#define I programIndex

typedef unsigned int16 half;

struct half2  { half x, y; };
struct half3  { half x, y, z; };
struct half4  { half x, y, z, w; };
struct quath  { half x, y, z, w; };
struct float2 { float x, y; };
struct float3 { float x, y, z; };
struct float4 { float x, y, z, w; };
struct quatf  { float x, y, z, w; };



float2 half_to_float(half2 h)
{
    float2 ret = {half_to_float(h.x), half_to_float(h.y)};
    return ret;
}
float3 half_to_float(half3 h)
{
    float3 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z)};
    return ret;
}
float4 half_to_float(half4 h)
{
    float4 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z), half_to_float(h.w)};
    return ret;
}


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

export uniform float3 Min(uniform const float3 src[], uniform const int num)
{
    uniform float3 rmin = src[0];

    const uniform int num_loops = num / C;
    if(num_loops > 1) {
        const uniform float * uniform fv = (const uniform float * uniform)src;
        uniform float tmin[3][C];
        tmin[0][I] = fv[C*0 + I];
        tmin[1][I] = fv[C*1 + I];
        tmin[2][I] = fv[C*2 + I];

        for(uniform int i=1; i < num_loops; ++i) {
            uniform const int i3 = i*3;

            float _0 = fv[C*(i3+0) + I];
            tmin[0][I] = min(tmin[0][I], _0);

            float _1 = fv[C*(i3+1) + I];
            tmin[1][I] = min(tmin[1][I], _1);

            float _2 = fv[C*(i3+2) + I];
            tmin[2][I] = min(tmin[2][I], _2);
        }

        float x,y,z;
        aos_to_soa3((uniform float*)&tmin[0], &x, &y, &z);
        rmin.x = reduce_min(x);
        rmin.y = reduce_min(y);
        rmin.z = reduce_min(z);
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        uniform float3 t = src[i];
        rmin.x = min(rmin.x, t.x);
        rmin.y = min(rmin.y, t.y);
        rmin.z = min(rmin.z, t.z);
    }

    return rmin;
}

export uniform float3 Max(uniform const float3 src[], uniform const int num)
{
    uniform float3 rmax = src[0];

    const uniform int num_loops = num / C;
    if(num_loops > 1) {
        const uniform float * uniform fv = (const uniform float * uniform)src;
        uniform float tmax[3][C];
        tmax[0][I] = fv[C*0 + I];
        tmax[1][I] = fv[C*1 + I];
        tmax[2][I] = fv[C*2 + I];

        for(uniform int i=1; i < num_loops; ++i) {
            uniform const int i3 = i*3;

            float _0 = fv[C*(i3+0) + I];
            tmax[0][I] = max(tmax[0][I], _0);

            float _1 = fv[C*(i3+1) + I];
            tmax[1][I] = max(tmax[1][I], _1);

            float _2 = fv[C*(i3+2) + I];
            tmax[2][I] = max(tmax[2][I], _2);
        }

        float x,y,z;
        aos_to_soa3((uniform float*)&tmax[0], &x, &y, &z);
        rmax.x = reduce_max(x);
        rmax.y = reduce_max(y);
        rmax.z = reduce_max(z);
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        uniform float3 t = src[i];
        rmax.x = max(rmax.x, t.x);
        rmax.y = max(rmax.y, t.y);
        rmax.z = max(rmax.z, t.z);
    }

    return rmax;
}

export void MinMax(
    uniform const float3 src[], uniform const int num,
    uniform float3& dst_min, uniform float3& dst_max)
{
    if(num == 0) { return; }

    uniform float3 rmin = src[0], rmax = src[0];

    const uniform int num_loops = num / C;
    if(num_loops > 1) {
        const uniform float * uniform fv = (const uniform float * uniform)src;
        uniform float tmin[3][C];
        uniform float tmax[3][C];
        tmin[0][I] = tmax[0][I] = fv[C*0 + I];
        tmin[1][I] = tmax[1][I] = fv[C*1 + I];
        tmin[2][I] = tmax[2][I] = fv[C*2 + I];

        for(uniform int i=1; i < num_loops; ++i) {
            uniform const int i3 = i*3;

            float _0 = fv[C*(i3+0) + I];
            tmin[0][I] = min(tmin[0][I], _0);
            tmax[0][I] = max(tmax[0][I], _0);

            float _1 = fv[C*(i3+1) + I];
            tmin[1][I] = min(tmin[1][I], _1);
            tmax[1][I] = max(tmax[1][I], _1);

            float _2 = fv[C*(i3+2) + I];
            tmin[2][I] = min(tmin[2][I], _2);
            tmax[2][I] = max(tmax[2][I], _2);
        }

        float x,y,z;
        aos_to_soa3((uniform float*)&tmin[0], &x, &y, &z);
        rmin.x = reduce_min(x);
        rmin.y = reduce_min(y);
        rmin.z = reduce_min(z);

        aos_to_soa3((uniform float*)&tmax[0], &x, &y, &z);
        rmax.x = reduce_max(x);
        rmax.y = reduce_max(y);
        rmax.z = reduce_max(z);
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        uniform float3 t = src[i];
        rmin.x = min(rmin.x, t.x);
        rmin.y = min(rmin.y, t.y);
        rmin.z = min(rmin.z, t.z);
        rmax.x = max(rmax.x, t.x);
        rmax.y = max(rmax.y, t.y);
        rmax.z = max(rmax.z, t.z);
    }

    dst_min = rmin;
    dst_max = rmax;
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







static inline float2 operator+(float2 a, float2 b) { float2 r = { a.x + b.x, a.y + b.y }; return r; }
static inline float2 operator-(float2 a, float2 b) { float2 r = { a.x - b.x, a.y - b.y }; return r; }
static inline float2 operator*(float2 a, float2 b) { float2 r = { a.x * b.x, a.y * b.y }; return r; }
static inline float2 operator/(float2 a, float2 b) { float2 r = { a.x / b.x, a.y / b.y }; return r; }
static inline float2 operator+(float2 a, float b) { float2 r = { a.x + b, a.y + b }; return r; }
static inline float2 operator-(float2 a, float b) { float2 r = { a.x - b, a.y - b }; return r; }
static inline float2 operator*(float2 a, float b) { float2 r = { a.x * b, a.y * b }; return r; }
static inline float2 operator/(float2 a, float b) { float2 r = { a.x / b, a.y / b }; return r; }
static inline float2 operator+(float a, float2 b) { float2 r = { a + b.x, a + b.y }; return r; }
static inline float2 operator-(float a, float2 b) { float2 r = { a - b.x, a - b.y }; return r; }
static inline float2 operator*(float a, float2 b) { float2 r = { a * b.x, a * b.y }; return r; }
static inline float2 operator/(float a, float2 b) { float2 r = { a / b.x, a / b.y }; return r; }

static inline float3 operator+(float3 a, float3 b) { float3 r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline float3 operator-(float3 a, float3 b) { float3 r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline float3 operator*(float3 a, float3 b) { float3 r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline float3 operator/(float3 a, float3 b) { float3 r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }
static inline float3 operator+(float3 a, float b) { float3 r = { a.x + b, a.y + b, a.z + b }; return r; }
static inline float3 operator-(float3 a, float b) { float3 r = { a.x - b, a.y - b, a.z - b }; return r; }
static inline float3 operator*(float3 a, float b) { float3 r = { a.x * b, a.y * b, a.z * b }; return r; }
static inline float3 operator/(float3 a, float b) { float3 r = { a.x / b, a.y / b, a.z / b }; return r; }
static inline float3 operator+(float a, float3 b) { float3 r = { a + b.x, a + b.y, a + b.z }; return r; }
static inline float3 operator-(float a, float3 b) { float3 r = { a - b.x, a - b.y, a - b.z }; return r; }
static inline float3 operator*(float a, float3 b) { float3 r = { a * b.x, a * b.y, a * b.z }; return r; }
static inline float3 operator/(float a, float3 b) { float3 r = { a / b.x, a / b.y, a / b.z }; return r; }

static inline uniform float2 operator+(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x + b.x, a.y + b.y }; return r; }
static inline uniform float2 operator-(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x - b.x, a.y - b.y }; return r; }
static inline uniform float2 operator*(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x * b.x, a.y * b.y }; return r; }
static inline uniform float2 operator/(uniform float2 a, uniform float2 b) { uniform float2 r = { a.x / b.x, a.y / b.y }; return r; }
static inline uniform float2 operator+(uniform float2 a, uniform float b) { uniform float2 r = { a.x + b, a.y + b }; return r; }
static inline uniform float2 operator-(uniform float2 a, uniform float b) { uniform float2 r = { a.x - b, a.y - b }; return r; }
static inline uniform float2 operator*(uniform float2 a, uniform float b) { uniform float2 r = { a.x * b, a.y * b }; return r; }
static inline uniform float2 operator/(uniform float2 a, uniform float b) { uniform float2 r = { a.x / b, a.y / b }; return r; }
static inline uniform float2 operator+(uniform float a, uniform float2 b) { uniform float2 r = { a + b.x, a + b.y }; return r; }
static inline uniform float2 operator-(uniform float a, uniform float2 b) { uniform float2 r = { a - b.x, a - b.y }; return r; }
static inline uniform float2 operator*(uniform float a, uniform float2 b) { uniform float2 r = { a * b.x, a * b.y }; return r; }
static inline uniform float2 operator/(uniform float a, uniform float2 b) { uniform float2 r = { a / b.x, a / b.y }; return r; }

static inline uniform float3 operator+(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline uniform float3 operator-(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline uniform float3 operator*(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline uniform float3 operator/(uniform float3 a, uniform float3 b) { uniform float3 r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }
static inline uniform float3 operator+(uniform float3 a, uniform float b) { uniform float3 r = { a.x + b, a.y + b, a.z + b }; return r; }
static inline uniform float3 operator-(uniform float3 a, uniform float b) { uniform float3 r = { a.x - b, a.y - b, a.z - b }; return r; }
static inline uniform float3 operator*(uniform float3 a, uniform float b) { uniform float3 r = { a.x * b, a.y * b, a.z * b }; return r; }
static inline uniform float3 operator/(uniform float3 a, uniform float b) { uniform float3 r = { a.x / b, a.y / b, a.z / b }; return r; }
static inline uniform float3 operator+(uniform float a, uniform float3 b) { uniform float3 r = { a + b.x, a + b.y, a + b.z }; return r; }
static inline uniform float3 operator-(uniform float a, uniform float3 b) { uniform float3 r = { a - b.x, a - b.y, a - b.z }; return r; }
static inline uniform float3 operator*(uniform float a, uniform float3 b) { uniform float3 r = { a * b.x, a * b.y, a * b.z }; return r; }
static inline uniform float3 operator/(uniform float a, uniform float3 b) { uniform float3 r = { a / b.x, a / b.y, a / b.z }; return r; }

static inline uniform float2 reduce_add(float2 v)
{
    uniform float2 r = { reduce_add(v.x), reduce_add(v.y) };
    return r;
}
static inline uniform float3 reduce_add(float3 v)
{
    uniform float3 r = {reduce_add(v.x), reduce_add(v.y), reduce_add(v.z)};
    return r;
}

static inline float mod(float a, float b)
{
    return a - b * floor(a / b);
}
static inline uniform float mod(uniform float a, uniform float b)
{
    return a - b * floor(a / b);
}

static inline float frac(float a)
{
    return mod(a, 1.0);
}
static inline uniform float frac(uniform float a)
{
    return mod(a, 1.0);
}


#define define_vmath1(f)\
    static inline float2 f(float2 a)\
    {\
        float2 r = { f(a.x), f(a.y) };\
        return r;\
    }\
    static inline uniform float2 f(uniform float2 a)\
    {\
        uniform float2 r = { f(a.x), f(a.y) };\
        return r;\
    }\
    static inline float3 f(float3 a)\
    {\
        float3 r = { f(a.x), f(a.y), f(a.z) };\
        return r;\
    }\
    static inline uniform float3 f(uniform float3 a)\
    {\
        uniform float3 r = { f(a.x), f(a.y), f(a.z) };\
        return r;\
    }

#define define_vmath2(f)\
    static inline float2 f(float2 a, float2 b)\
    {\
        float2 r = { f(a.x, b.x), f(a.y, b.y) };\
        return r;\
    }\
    static inline uniform float2 f(uniform float2 a, uniform float2 b)\
    {\
        uniform float2 r = { f(a.x, b.x), f(a.y, b.y) };\
        return r;\
    }\
    static inline float3 f(float3 a, float3 b)\
    {\
        float3 r = { f(a.x, b.x), f(a.y, b.y), f(a.z, b.z) };\
        return r;\
    }\
    static inline uniform float3 f(uniform float3 a, uniform float3 b)\
    {\
        uniform float3 r = { f(a.x, b.x), f(a.y, b.y), f(a.z, b.z) };\
        return r;\
    }

define_vmath1(abs)
define_vmath1(round)
define_vmath1(floor)
define_vmath1(ceil)
define_vmath2(min)
define_vmath2(max)
define_vmath1(rcp)
define_vmath1(sqrt)
define_vmath1(rsqrt)
define_vmath1(sin)
define_vmath1(cos)
define_vmath1(tan)
define_vmath1(asin)
define_vmath1(acos)
define_vmath1(atan)
define_vmath2(atan2)
define_vmath1(exp)
define_vmath1(log)
define_vmath2(pow)
define_vmath2(mod)
define_vmath1(frac)



static inline float dot(float2 a, float2 b)
{
    return a.x*b.x + a.y*b.y;
}
static inline uniform float dot(uniform float2 a, uniform float2 b)
{
    return a.x*b.x + a.y*b.y;
}
static inline float dot(float3 a, float3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static inline uniform float dot(uniform float3 a, uniform float3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


static inline float3 cross(float3 v0, float3 v1)
{
    float3 ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}
static inline uniform float3 cross(uniform float3 v0, uniform float3 v1)
{
    uniform float3 ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}



static inline float length_sq(float2 v)
{
    return dot(v, v);
}
static inline uniform float length_sq(uniform float2 v)
{
    return dot(v, v);
}
static inline float length_sq(float3 v)
{
    return dot(v, v);
}
static inline uniform float length_sq(uniform float3 v)
{
    return dot(v, v);
}

static inline float length(float2 v)
{
    return sqrt(length_sq(v));
}
static inline uniform float length(uniform float2 v)
{
    return sqrt(length_sq(v));
}
static inline float length(float3 v)
{
    return sqrt(length_sq(v));
}
static inline uniform float length(uniform float3 v)
{
    return sqrt(length_sq(v));
}

// somehow length_est() is slower than length()...
static inline float length_est(float3 v)
{
    float t = length_sq(v);
    return rsqrt(t)*t;
}
static inline uniform float length_est(uniform float3 v)
{
    uniform float t = length_sq(v);
    return rsqrt(t)*t;
}

static inline float rcp_length(float2 v)
{
    return rsqrt(length_sq(v));
}
static inline uniform float rcp_length(uniform float2 v)
{
    return rsqrt(length_sq(v));
}
static inline float rcp_length(float3 v)
{
    return rsqrt(length_sq(v));
}
static inline uniform float rcp_length(uniform float3 v)
{
    return rsqrt(length_sq(v));
}

static inline float2 normalize(float2 v)
{
    return v * rsqrt(dot(v, v));
}
static inline uniform float2 normalize(uniform float2 v)
{
    return v * rsqrt(dot(v, v));
}
static inline float3 normalize(float3 v)
{
    return v * rsqrt(dot(v, v));
}
static inline uniform float3 normalize(uniform float3 v)
{
    return v * rsqrt(dot(v, v));
}

static inline float lerp(float a, float b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform float lerp(uniform float a, uniform float b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline float3 lerp(float3 a, float3 b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform float3 lerp(uniform float3 a, uniform float3 b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline float clamp_and_normalize(float v, float low, float high, float rcp_range) {
    float r = (v - low)*rcp_range;
    return clamp(r, 0.0f, 1.0f);
}

#define Epsilon 0.0001f

export uniform int RayTrianglesIntersection(
    uniform const float3 pos_[], uniform const float3 dir_[],
    uniform const float3 vertices[], uniform const int indices[], uniform const int num_triangles,
    uniform int result[])
{
    uniform float3 pos = *pos_;
    uniform float3 dir = *dir_;
    uniform int total_hit = 0;

    // SIMD pass
    uniform int num_triangles_simd = num_triangles & ~(C - 1);
    for(uniform int bi=0; bi < num_triangles_simd; bi += C) {
        uniform float3 va1[C];
        uniform float3 va2[C];
        uniform float3 va3[C];
        for(uniform int i = 0; i < C; ++i) {
            uniform int ti = bi + i;
            va1[i] = vertices[indices[ti * 3 + 0]];
            va2[i] = vertices[indices[ti * 3 + 1]];
            va3[i] = vertices[indices[ti * 3 + 2]];
        }

        float3 p1;
        float3 p2;
        float3 p3;
        aos_to_soa3((uniform float*)&va1[0], &p1.x, &p1.y, &p1.z);
        aos_to_soa3((uniform float*)&va2[0], &p2.x, &p2.y, &p2.z);
        aos_to_soa3((uniform float*)&va3[0], &p3.x, &p3.y, &p3.z);

        int hit = true;
        float3 e1 = p2 - p1;
        float3 e2 = p3 - p1;
        float3 p = cross(dir, e2);
        float det = dot(e1, p);
        if (det > -Epsilon && det < Epsilon) { hit = false; }
        float invDet = 1.0f / det;
        float3 t = pos - p1;
        float u = dot(t, p) * invDet;
        if (u < 0 || u > 1) { hit = false; }

        float3 q = cross(t, e1);
        float v = dot(dir, q) * invDet;
        if (v < 0 || u + v > 1) {  hit = false; }

        if ((dot(e2, q) * invDet) > Epsilon) {
            // hit
        }
        else {
            hit = false;
        }

        uniform int hita[C];
        packed_store_active(hita, hit);
        for(uniform int i = 0; i < C; ++i) {
            if(hita[i]) {
                result[total_hit++] = bi + i;
            }
        }
    }

    // non-SIMD pass
    for(uniform int ti = num_triangles_simd; ti < num_triangles; ++ti) {
        uniform float3 p1 = vertices[indices[ti * 3 + 0]];
        uniform float3 p2 = vertices[indices[ti * 3 + 1]];
        uniform float3 p3 = vertices[indices[ti * 3 + 2]];

        uniform bool hit = true;
        uniform float3 e1 = p2 - p1;
        uniform float3 e2 = p3 - p1;
        uniform float3 p = cross(dir, e2);
        uniform float det = dot(e1, p);
        if (det > -Epsilon && det < Epsilon) { hit = false; }
        uniform float invDet = 1.0f / det;
        uniform float3 t = pos - p1;
        uniform float u = dot(t, p) * invDet;
        if (u < 0 || u > 1) { hit = false; }

        uniform float3 q = cross(t, e1);
        uniform float v = dot(dir, q) * invDet;
        if (v < 0 || u + v > 1) {  hit = false; }

        if ((dot(e2, q) * invDet) > Epsilon) {
            // hit
        }
        else {
            hit = false;
        }

        if(hit) {
            result[total_hit++] = ti;
        }
    }

    return total_hit;
}
