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



static inline float2 half_to_float(half2 h)
{
    float2 ret = {half_to_float(h.x), half_to_float(h.y)};
    return ret;
}
static inline float3 half_to_float(half3 h)
{
    float3 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z)};
    return ret;
}
static inline float4 half_to_float(half4 h)
{
    float4 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z), half_to_float(h.w)};
    return ret;
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



#define Epsilon 1e-6

static inline bool TriangleIntersection(uniform float3 pos, uniform float3 dir, float3 p1, float3 p2, float3 p3, float& distance)
{
    float3 e1 = p2 - p1;
    float3 e2 = p3 - p1;
    float3 p = cross(dir, e2);
    float det = dot(e1, p);
    float inv_det = 1.0f / det;
    float3 t = pos - p1;
    float u = dot(t, p) * inv_det;
    float3 q = cross(t, e1);
    float v = dot(dir, q) * inv_det;

    distance = dot(e2, q) * inv_det;
    return
        abs(det) > Epsilon &&
        u >= 0 && u <= 1 &&
        v >= 0 && u + v <= 1;
}

// uniform variant
static inline uniform bool TriangleIntersection(uniform float3 pos, uniform float3 dir, uniform float3 p1, uniform float3 p2, uniform float3 p3, uniform float& distance)
{
    uniform float3 e1 = p2 - p1;
    uniform float3 e2 = p3 - p1;
    uniform float3 p = cross(dir, e2);
    uniform float det = dot(e1, p);
    uniform float inv_det = 1.0f / det;
    uniform float3 t = pos - p1;
    uniform float u = dot(t, p) * inv_det;
    uniform float3 q = cross(t, e1);
    uniform float v = dot(dir, q) * inv_det;

    distance = dot(e2, q) * inv_det;
    return
        abs(det) > Epsilon &&
        u >= 0 && u <= 1 &&
        v >= 0 && u + v <= 1;
}

