#pragma once

#include <cmath>
#include <cstring>
#include <algorithm>
#ifdef muEnableHalf
    #include "half.h"
#endif // muEnableHalf

#define muDefaultEpsilon 0.00001f
#define muMath_AddNamespace

#ifdef muMath_AddNamespace
namespace mu {
#endif

extern const float PI;
extern const float Deg2Rad;
extern const float Rad2Deg;

struct float2
{
    float x, y;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
    bool operator==(const float2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const float2& v) const { return !((*this)==v); }

    void assign(const float *v)
    {
        *this = { v[0], v[1] };
    }
    void assign(const double *v)
    {
        *this = { (float)v[0], (float)v[1] };
    }

    static float2 zero() { return{ 0.0f, 0.0f }; }
    static float2 one() { return{ 1.0f, 1.0f }; }
};
struct float3
{
    float x, y, z;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
    bool operator==(const float3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const float3& v) const { return !((*this) == v); }

    void assign(const float *v)
    {
        *this = { v[0], v[1], v[2] };
    }
    void assign(const double *v)
    {
        *this = { (float)v[0], (float)v[1], (float)v[2] };
    }

    static float3 zero() { return{ 0.0f, 0.0f, 0.0f }; }
    static float3 one() { return{ 1.0f, 1.0f, 1.0f }; }
};
struct float4
{
    float x, y, z, w;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
    bool operator==(const float4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const float4& v) const { return !((*this) == v); }

    void assign(const float *v)
    {
        *this = { v[0], v[1], v[2], v[3] };
    }
    void assign(const double *v)
    {
        *this = { (float)v[0], (float)v[1], (float)v[2], (float)v[3] };
    }

    static float4 zero() { return{ 0.0f, 0.0f, 0.0f, 0.0f }; }
    static float4 one() { return{ 1.0f, 1.0f, 1.0f, 1.0f }; }
};
struct quatf
{
    float x, y, z, w;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
    bool operator==(const quatf& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const quatf& v) const { return !((*this) == v); }

    void assign(const float *v)
    {
        *this = { v[0], v[1], v[2], v[3] };
    }
    void assign(const double *v)
    {
        *this = { (float)v[0], (float)v[1], (float)v[2], (float)v[3] };
    }

    static quatf identity() { return{ 0.0f, 0.0f, 0.0f, 1.0f }; }
};

struct float3x3
{
    float3 m[3];
    float3& operator[](int i) { return m[i]; }
    const float3& operator[](int i) const { return m[i]; }
    bool operator==(const float3x3& v) const { return memcmp(m, v.m, sizeof(*this)) == 0; }
    bool operator!=(const float3x3& v) const { return !((*this) == v); }

    void assign(const float *v)
    {
        memcpy(this, v, sizeof(*this));
    }
    void assign(const double *v)
    {
        *this = { {
            { (float)v[0], (float)v[1], (float)v[2] },
            { (float)v[3], (float)v[4], (float)v[5] },
            { (float)v[6], (float)v[7], (float)v[8] }
        } };
    }

    static float3x3 identity()
    {
        return{ {
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
        } };
    }
};
struct float4x4 
{
    float4 m[4];
    float4& operator[](int i) { return m[i]; }
    const float4& operator[](int i) const { return m[i]; }
    bool operator==(const float4x4& v) const { return memcmp(m, v.m, sizeof(*this)) == 0; }
    bool operator!=(const float4x4& v) const { return !((*this) == v); }

    void assign(const float *v)
    {
        memcpy(this, v, sizeof(*this));
    }
    void assign(const double *v)
    {
        *this = { {
            { (float)v[0], (float)v[1], (float)v[2], (float)v[3] },
            { (float)v[4], (float)v[5], (float)v[6], (float)v[7] },
            { (float)v[8], (float)v[9], (float)v[10],(float)v[11]},
            { (float)v[12],(float)v[13],(float)v[14],(float)v[15]}
        } };
    }

    static float4x4 identity()
    {
        return{ {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        } };
    }
};


inline bool near_equal(float a, float b, float epsilon = muDefaultEpsilon)
{
    return std::abs(a - b) < epsilon;
}
inline bool near_equal(const float2& a, const float2& b, float e = muDefaultEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e);
}
inline bool near_equal(const float3& a, const float3& b, float e = muDefaultEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e);
}
inline bool near_equal(const float4& a, const float4& b, float e = muDefaultEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e) && near_equal(a.w, b.w, e);
}
inline bool near_equal(const quatf& a, const quatf& b, float e = muDefaultEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e) && near_equal(a.w, b.w, e);
}
inline bool near_equal(const float3x3& a, const float3x3& b, float e = muDefaultEpsilon)
{
    return near_equal(a[0], b[0], e) && near_equal(a[1], b[1], e) && near_equal(a[2], b[2], e);
}
inline bool near_equal(const float4x4& a, const float4x4& b, float e = muDefaultEpsilon)
{
    return near_equal(a[0], b[0], e) && near_equal(a[1], b[1], e) && near_equal(a[2], b[2], e) && near_equal(a[3], b[3], e);
}

template<class Int>
inline Int ceildiv(Int v, Int d)
{
    return v / d + (v % d == 0 ? 0 : 1);
}


inline float2 operator+(const float2& l, const float2& r)
{
    return{ l.x + r.x, l.y + r.y };
}
inline float2 operator-(const float2& v)
{
    return{ -v.x, -v.y };
}
inline float2 operator-(const float2& l, const float2& r)
{
    return{ l.x - r.x, l.y - r.y };
}
inline float2 operator*(const float2& l, float r)
{
    return{ l.x * r, l.y * r };
}
inline float2 operator*(const float2& l, const float2& r)
{
    return{ l.x * r.x, l.y * r.y };
}
inline float2 operator/(const float2& l, float r)
{
    return{ l.x / r, l.y / r };
}
inline float2 operator/(const float2& l, const float2& r)
{
    return{ l.x / r.x, l.y / r.y };
}

inline float2& operator+=(float2& l, const float2& r)
{
    l = l + r;
    return l;
}
inline float2& operator-=(float2& l, const float2& r)
{
    l = l - r;
    return l;
}
inline float2& operator*=(float2& l, float r)
{
    l = l * r;
    return l;
}
inline float2& operator*=(float2& l, const float2& r)
{
    l = l * r;
    return l;
}
inline float2& operator/=(float2& l, float r)
{
    l = l / r;
    return l;
}
inline float2& operator/=(float2& l, const float2& r)
{
    l = l / r;
    return l;
}

inline float3 operator+(const float3& l, const float3& r)
{
    return{ l.x + r.x, l.y + r.y, l.z + r.z };
}
inline float3 operator-(const float3& v)
{
    return{-v.x, -v.y, -v.z};
}
inline float3 operator-(const float3& l, const float3& r)
{
    return{ l.x - r.x, l.y - r.y, l.z - r.z };
}
inline float3 operator*(const float3& l, float r)
{
    return{ l.x * r, l.y * r, l.z * r };
}
inline float3 operator*(const float3& l, const float3& r)
{
    return{ l.x * r.x, l.y * r.y, l.z * r.z };
}
inline float3 operator/(const float3& l, float r)
{
    return{ l.x / r, l.y / r, l.z / r };
}
inline float3 operator/(const float3& l, const float3& r)
{
    return{ l.x / r.x, l.y / r.y, l.z / r.z };
}

inline float3& operator+=(float3& l, const float3& r)
{
    l = l + r;
    return l;
}
inline float3& operator-=(float3& l, const float3& r)
{
    l = l - r;
    return l;
}
inline float3& operator*=(float3& l, float r)
{
    l = l * r;
    return l;
}
inline float3& operator*=(float3& l, const float3& r)
{
    l = l * r;
    return l;
}
inline float3& operator/=(float3& l, float r)
{
    l = l / r;
    return l;
}
inline float3& operator/=(float3& l, const float3& r)
{
    l = l / r;
    return l;
}


inline float4 operator*(const float4& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r, l.w*r };
}
inline float4& operator*=(float4& l, float r)
{
    l = l * r;
    return l;
}

inline quatf operator*(const quatf& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r, l.w*r };
}

inline quatf operator*(const quatf& l, const quatf& r)
{
    return{
        l.w*r.x + l.x*r.w + l.y*r.z - l.z*r.y,
        l.w*r.y + l.y*r.w + l.z*r.x - l.x*r.z,
        l.w*r.z + l.z*r.w + l.x*r.y - l.y*r.x,
        l.w*r.w - l.x*r.x - l.y*r.y - l.z*r.z,
    };
}

inline quatf& operator*=(quatf& l, float r)
{
    l = l * r;
    return l;
}
inline quatf& operator*=(quatf& l, const quatf& r)
{
    l = l * r;
    return l;
}

inline float3 operator*(const float3x3& m, const float3& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
inline float3 operator*(const float4x4& m, const float3& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
inline float4 operator*(const float4x4& m, const float4& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3],
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3],
    };
}

inline float4x4 operator*(const float4x4 &a, const float4x4 &b)
{
    float4x4 c;
    const float *ap = &a[0][0];
    const float *bp = &b[0][0];
          float *cp = &c[0][0];
    float a0, a1, a2, a3;

    a0 = ap[0];
    a1 = ap[1];
    a2 = ap[2];
    a3 = ap[3];

    cp[0] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[1] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[2] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[3] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

    a0 = ap[4];
    a1 = ap[5];
    a2 = ap[6];
    a3 = ap[7];

    cp[4] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[5] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[6] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[7] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

    a0 = ap[8];
    a1 = ap[9];
    a2 = ap[10];
    a3 = ap[11];

    cp[8] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[9] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[10] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[11] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

    a0 = ap[12];
    a1 = ap[13];
    a2 = ap[14];
    a3 = ap[15];

    cp[12] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
    cp[13] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
    cp[14] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
    cp[15] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];
    return c;
}
inline float4x4& operator*=(float4x4& a, const float4x4 &b)
{
    a = a * b;
    return a;
}

inline float3 applyTRS(const float4x4& m, const float3& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2],
    };
}

inline float dot(const float3& l, const float3& r)
{
    return l.x*r.x + l.y*r.y + l.z*r.z;
}

inline float3 normalize(const float3& l)
{
    float d = 1.0f / std::sqrt(dot(l, l));
    return l * d;
}

inline float3 cross(const float3& l, const float3& r)
{
    return{
        l.y * r.z - l.z * r.y,
        l.z * r.x - l.x * r.z,
        l.x * r.y - l.y * r.x };
}

inline float3 apply_rotation(const quatf& q, const float3& p)
{
    float3 a = cross((float3&)q, p);
    float3 b = cross((float3&)q, a);
    return p + (a * q.w + b) * 2.0f;
}

inline quatf inverse(const quatf& v)
{
    return{ -v.x, -v.y, -v.z, v.w };
}

inline quatf flipY(const quatf& v)
{
    return{ -v.z, v.w, v.x, -v.y, };
}

inline quatf rotateX(float angle)
{
    float c = std::cos(angle * 0.5f);
    float s = std::sin(angle * 0.5f);
    return{ s, 0.0f, 0.0f, c };
}
inline quatf rotateY(float angle)
{
    float c = std::cos(angle * 0.5f);
    float s = std::sin(angle * 0.5f);
    return{ 0.0f, s, 0.0f, c };
}
inline quatf rotateZ(float angle)
{
    float c = std::cos(angle * 0.5f);
    float s = std::sin(angle * 0.5f);
    return{ 0.0f, 0.0f, s, c };
}

// eular -> quaternion
inline quatf rotateXYZ(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qZ * qY) * qX;
}
inline quatf rotateXZY(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qY * qZ) * qX;
}
inline quatf rotateYXZ(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qZ * qX) * qY;
}
inline quatf rotateYZX(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qX * qZ) * qY;
}
inline quatf rotateZXY(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qY * qX) * qZ;
}
inline quatf rotateZYX(const float3& euler)
{
    quatf qX = rotateX(euler.x);
    quatf qY = rotateY(euler.y);
    quatf qZ = rotateZ(euler.z);
    return (qX * qY) * qZ;
}

inline quatf rotate(const float3& axis, float angle)
{
    return{
        axis.x * std::sin(angle * 0.5f),
        axis.y * std::sin(angle * 0.5f),
        axis.z * std::sin(angle * 0.5f),
        std::cos(angle * 0.5f)
    };
}

template<class T> inline float clamp(T v, T vmin, T vmax) { return std::min<T>(std::max<T>(v, vmin), vmax); }
inline float saturate(float v) { return clamp(v, -1.0f, 1.0f); }

inline float3 eularZXY(const quatf& q)
{
    float d[] = {
        q.x*q.x, q.x*q.y, q.x*q.z, q.x*q.w,
        q.y*q.y, q.y*q.z, q.y*q.w,
        q.z*q.z, q.z*q.w,
        q.w*q.w
    };

    float v0 = d[5] - d[3];
    float v1 = 2.0f*(d[1] + d[8]);
    float v2 = d[4] - d[7] - d[0] + d[9];
    float v3 = -1.0f;
    float v4 = 2.0f * v0;

    const float SINGULARITY_CUTOFF = 0.499999f;
    if (std::abs(v0) < SINGULARITY_CUTOFF)
    {
        float v5 = 2.0f * (d[2] + d[6]);
        float v6 = d[7] - d[0] - d[4] + d[9];

        return{
            v3 * std::asin(saturate(v4)),
            std::atan2(v5, v6),
            std::atan2(v1, v2)
        };
    }
    else
    {
        float a = d[1] + d[8];
        float b =-d[5] + d[3];
        float c = d[1] - d[8];
        float e = d[5] + d[3];

        float v5 = a*e + b*c;
        float v6 = b*e - a*c;

        return{
            v3 * std::asin(saturate(v4)),
            std::atan2(v5, v6),
            0.0f
        };
    }
}

inline quatf swap_handedness(const quatf& q)
{
    return { q.x, -q.y, -q.z, q.w };
}

inline float3x3 swap_handedness(const float3x3& m)
{
    return{ {
        { m[0].x,-m[0].y,-m[0].z },
        {-m[1].x, m[1].y, m[1].z },
        {-m[2].x, m[2].y, m[2].z },
    } };
}

inline float4x4 swap_handedness(const float4x4& m)
{
    return{ {
        { m[0].x,-m[0].y,-m[0].z, m[0].w },
        {-m[1].x, m[1].y, m[1].z, m[1].w },
        {-m[2].x, m[2].y, m[2].z, m[2].w },
        {-m[3].x, m[3].y, m[3].z, m[3].w },
    } };
}

inline float3x3 to_float3x3(const quatf& q)
{
    return {{
        {1.0f-2.0f*q.y*q.y - 2.0f*q.z*q.z, 2.0f*q.x*q.y - 2.0f*q.z*q.w,        2.0f*q.x*q.z + 2.0f*q.y*q.w,      },
        {2.0f*q.x*q.y + 2.0f*q.z*q.w,      1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z, 2.0f*q.y*q.z - 2.0f*q.x*q.w,      },
        {2.0f*q.x*q.z - 2.0f*q.y*q.w,      2.0f*q.y*q.z + 2.0f*q.x*q.w,        1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y}
    }};
}
inline float4x4 to_float4x4(const quatf& q)
{
    return {{
        {1.0f-2.0f*q.y*q.y - 2.0f*q.z*q.z, 2.0f*q.x*q.y - 2.0f*q.z*q.w,        2.0f*q.x*q.z + 2.0f*q.y*q.w,        0.0},
        {2.0f*q.x*q.y + 2.0f*q.z*q.w,      1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z, 2.0f*q.y*q.z - 2.0f*q.x*q.w,        0.0},
        {2.0f*q.x*q.z - 2.0f*q.y*q.w,      2.0f*q.y*q.z + 2.0f*q.x*q.w,        1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y, 0.0},
        {0.0,                              0.0,                                0.0,                                1.0}
    }};
}

inline float4x4 translate(const float3& t)
{
    return {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        {  t.x,  t.y,  t.z, 1.0f }
    } };
}

inline float4x4 scale(const float3& t)
{
    return{ {
        {  t.x, 0.0f, 0.0f, 0.0f },
        { 0.0f,  t.y, 0.0f, 0.0f },
        { 0.0f, 0.0f,  t.z, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    } };
}

inline float4x4 transform(const float3& t, const quatf& r, const float3& s)
{
    auto ret = scale(s);
    ret *= to_float4x4(r);
    ret *= translate(t);
    return ret;
}

inline float4x4 invert(const float4x4& x)
{
    float4x4 s = {
        x[1][1] * x[2][2] - x[2][1] * x[1][2],
        x[2][1] * x[0][2] - x[0][1] * x[2][2],
        x[0][1] * x[1][2] - x[1][1] * x[0][2],
        0,

        x[2][0] * x[1][2] - x[1][0] * x[2][2],
        x[0][0] * x[2][2] - x[2][0] * x[0][2],
        x[1][0] * x[0][2] - x[0][0] * x[1][2],
        0,

        x[1][0] * x[2][1] - x[2][0] * x[1][1],
        x[2][0] * x[0][1] - x[0][0] * x[2][1],
        x[0][0] * x[1][1] - x[1][0] * x[0][1],
        0,

        0,
        0,
        0,
        1,
    };

    auto r = x[0][0] * s[0][0] + x[0][1] * s[1][0] + x[0][2] * s[2][0];

    if (std::abs(r) >= 1)
    {
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                s[i][j] /= r;
            }
        }
    }
    else
    {
        auto mr = std::abs(r) / std::numeric_limits<float>::min();

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                if (mr > std::abs(s[i][j]))
                {
                    s[i][j] /= r;
                }
                else
                {
                    // error
                    return float4x4::identity();
                }
            }
        }
    }

    s[3][0] = -x[3][0] * s[0][0] - x[3][1] * s[1][0] - x[3][2] * s[2][0];
    s[3][1] = -x[3][0] * s[0][1] - x[3][1] * s[1][1] - x[3][2] * s[2][1];
    s[3][2] = -x[3][0] * s[0][2] - x[3][1] * s[1][2] - x[3][2] * s[2][2];
    return s;
}

template<class T>
inline quatf to_quat_impl(const T& m)
{
    float tr, s;
    float q[4];
    int i, j, k;
    quatf quat;

    int nxt[3] = { 1, 2, 0 };
    tr = m[0][0] + m[1][1] + m[2][2];

    // check the diagonal
    if (tr > 0.0) {
        s = std::sqrt(tr + 1.0f);
        quat.w = s / 2.0f;
        s = 0.5f / s;

        quat.x = (m[1][2] - m[2][1]) * s;
        quat.y = (m[2][0] - m[0][2]) * s;
        quat.z = (m[0][1] - m[1][0]) * s;
    }
    else {
        // diagonal is negative
        i = 0;
        if (m[1][1] > m[0][0])
            i = 1;
        if (m[2][2] > m[i][i])
            i = 2;

        j = nxt[i];
        k = nxt[j];
        s = std::sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

        q[i] = s * 0.5f;
        if (s != 0.0f)
            s = 0.5f / s;

        q[3] = (m[j][k] - m[k][j]) * s;
        q[j] = (m[i][j] + m[j][i]) * s;
        q[k] = (m[i][k] + m[k][i]) * s;

        quat.x = q[0];
        quat.y = q[1];
        quat.z = q[2];
        quat.w = q[3];
    }

    return quat;
}

inline quatf to_quat(const float3x3& m)
{
    return to_quat_impl(m);
}
inline quatf to_quat(const float4x4& m)
{
    return to_quat_impl(m);
}

// aperture and focal_length must be millimeter. return fov in degree
inline float compute_fov(float aperture, float focal_length)
{
    return 2.0f * atanf(aperture / (2.0f * focal_length)) * Rad2Deg;
}

// aperture: millimeter
// fov: degree
inline float compute_focal_length(float aperture, float fov)
{
    return aperture / std::tan(fov * Deg2Rad / 2.0f) / 2.0f;
}


inline bool ray_triangle_intersection(float3 pos, float3 dir, float3 p1, float3 p2, float3 p3, float2& uv)
{
    float3 e1 = p2 - p1;
    float3 e2 = p3 - p1;
    float3 p = cross(dir, e2);
    float det = dot(e1, p);
    if (det > -muDefaultEpsilon && det < muDefaultEpsilon) { return false; }
    float invDet = 1.0f / det;
    float3 t = pos - p1;
    float u = dot(t, p) * invDet;
    if (u < 0 || u > 1) { return false; }

    float3 q = cross(t, e1);
    float v = dot(dir, q) * invDet;
    if (v < 0 || u + v > 1) { return false; }

    if ((dot(e2, q) * invDet) > muDefaultEpsilon)
    {
        uv.x = u;
        uv.y = v;
        return true;
    }
    return false;
}

#ifdef muMath_AddNamespace
} // namespace mu
#endif
