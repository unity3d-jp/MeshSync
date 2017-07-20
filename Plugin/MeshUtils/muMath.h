#pragma once

#include <cmath>
#include <cstring>
#include <algorithm>
#ifdef muEnableHalf
    #include <OpenEXR/half.h>
#endif // muEnableHalf
#include "muIntrusiveArray.h"

#define muEpsilon 1e-4f

namespace mu {

extern const float PI;
extern const float Deg2Rad;
extern const float Rad2Deg;

template<class T>
struct tvec2
{
    using scalar_t = T;
    T x, y;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const tvec2& v) const { return !((*this)==v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1] }; }
    template<class U> void assign(const tvec2<U>& v) { assign((const U*)&v); }

    static tvec2 zero() { return{ (T)0, (T)0 }; }
    static tvec2 one() { return{ (T)1, (T)1 }; }
};

template<class T>
struct tvec3
{
    using scalar_t = T;
    T x, y, z;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const tvec3& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1], (T)v[2] }; }
    template<class U> void assign(const tvec3<U>& v) { assign((const U*)&v); }

    static tvec3 zero() { return{ (T)0, (T)0, (T)0 }; }
    static tvec3 one() { return{ (T)1, (T)1, (T)1 }; }
};

template<class T>
struct tvec4
{
    using scalar_t = T;
    T x, y, z, w;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const tvec4& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1], (T)v[2], (T)v[3] }; }
    template<class U> void assign(const tvec4<U>& v) { assign((const U*)&v); }

    static tvec4 zero() { return{ (T)0, (T)0, (T)0, (T)0 }; }
    static tvec4 one() { return{ (T)1, (T)1, (T)1, (T)1 }; }
};

template<class T>
struct tquat
{
    using scalar_t = T;
    T x, y, z, w;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tquat& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const tquat& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1], (T)v[2], (T)v[3] }; }
    template<class U> void assign(const tquat<U>& v) { assign((const U*)&v); }

    static tquat identity() { return{ (T)0, (T)0, (T)0, (T)1 }; }
};

template<class T>
struct tmat3x3
{
    using scalar_t = T;
    using vector_t = tvec3<T>;
    tvec3<T> m[3];
    tvec3<T>& operator[](int i) { return m[i]; }
    const tvec3<T>& operator[](int i) const { return m[i]; }
    bool operator==(const tmat3x3& v) const { return memcmp(m, v.m, sizeof(*this)) == 0; }
    bool operator!=(const tmat3x3& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v)
    {
        *this = { {
            { (T)v[0], (T)v[1], (T)v[2] },
            { (T)v[3], (T)v[4], (T)v[5] },
            { (T)v[6], (T)v[7], (T)v[8] }
        } };
    }
    template<class U> void assign(const tmat3x3<U>& v) { assign((U*)&v); }

    static tmat3x3 identity()
    {
        return{ {
            { T(1.0), T(0.0), T(0.0) },
            { T(0.0), T(1.0), T(0.0) },
            { T(0.0), T(0.0), T(1.0) },
        } };
    }
};

template<class T>
struct tmat4x4
{
    using scalar_t = T;
    using vector_t = tvec4<T>;
    tvec4<T> m[4];
    tvec4<T>& operator[](int i) { return m[i]; }
    const tvec4<T>& operator[](int i) const { return m[i]; }
    bool operator==(const tmat4x4& v) const { return memcmp(m, v.m, sizeof(*this)) == 0; }
    bool operator!=(const tmat4x4& v) const { return !((*this) == v); }

    void assign(const T *v)
    {
        memcpy(this, v, sizeof(*this));
    }
    template<class U> void assign(const U *v)
    {
        *this = { {
            { (T)v[0], (T)v[1], (T)v[2], (T)v[3] },
            { (T)v[4], (T)v[5], (T)v[6], (T)v[7] },
            { (T)v[8], (T)v[9], (T)v[10],(T)v[11]},
            { (T)v[12],(T)v[13],(T)v[14],(T)v[15]}
        } };
    }
    template<class U> void assign(const tmat4x4<U>& v) { assign((U*)&v); }

    static tmat4x4 identity()
    {
        return{ {
            { (T)1, (T)0, (T)0, (T)0 },
            { (T)0, (T)1, (T)0, (T)0 },
            { (T)0, (T)0, (T)1, (T)0 },
            { (T)0, (T)0, (T)0, (T)1 },
        } };
    }
};

#ifdef muEnableHalf
using half2 = tvec2<half>;
using half3 = tvec3<half>;
using half4 = tvec4<half>;
using quath = tquat<half>;
using half3x3 = tmat3x3<half>;
using half4x4 = tmat4x4<half>;
#endif

using float2 = tvec2<float>;
using float3 = tvec3<float>;
using float4 = tvec4<float>;
using quatf = tquat<float>;
using float3x3 = tmat3x3<float>;
using float4x4 = tmat4x4<float>;

using double2 = tvec2<double>;
using double3 = tvec3<double>;
using double4 = tvec4<double>;
using quatd = tquat<double>;
using double3x3 = tmat3x3<double>;
using double4x4 = tmat4x4<double>;

template<class T> inline tvec2<T> operator-(const tvec2<T>& v) { return{ -v.x, -v.y }; }
template<class T, class U> inline tvec2<T> operator+(const tvec2<T>& l, const tvec2<U>& r) { return{ l.x + r.x, l.y + r.y }; }
template<class T, class U> inline tvec2<T> operator-(const tvec2<T>& l, const tvec2<U>& r) { return{ l.x - r.x, l.y - r.y }; }
template<class T, class U> inline tvec2<T> operator*(const tvec2<T>& l, const tvec2<U>& r) { return{ l.x * r.x, l.y * r.y }; }
template<class T, class U> inline tvec2<T> operator/(const tvec2<T>& l, const tvec2<U>& r) { return{ l.x / r.x, l.y / r.y }; }
template<class T> inline tvec2<T> operator+(T l, const tvec2<T>& r) { return{ l + r.x, l + r.y }; }
template<class T> inline tvec2<T> operator-(T l, const tvec2<T>& r) { return{ l - r.x, l - r.y }; }
template<class T> inline tvec2<T> operator*(T l, const tvec2<T>& r) { return{ l * r.x, l * r.y }; }
template<class T> inline tvec2<T> operator/(T l, const tvec2<T>& r) { return{ l / r.x, l / r.y }; }
template<class T> inline tvec2<T> operator+(const tvec2<T>& l, T r) { return{ l.x + r, l.y + r }; }
template<class T> inline tvec2<T> operator-(const tvec2<T>& l, T r) { return{ l.x - r, l.y - r }; }
template<class T> inline tvec2<T> operator*(const tvec2<T>& l, T r) { return{ l.x * r, l.y * r }; }
template<class T> inline tvec2<T> operator/(const tvec2<T>& l, T r) { return{ l.x / r, l.y / r }; }
template<class T, class U> inline tvec2<T>& operator+=(tvec2<T>& l, const tvec2<U>& r) { l.x += r.x; l.y += r.y; return l; }
template<class T, class U> inline tvec2<T>& operator-=(tvec2<T>& l, const tvec2<U>& r) { l.x -= r.x; l.y -= r.y; return l; }
template<class T, class U> inline tvec2<T>& operator*=(tvec2<T>& l, const tvec2<U>& r) { l.x *= r.x; l.y *= r.y; return l; }
template<class T, class U> inline tvec2<T>& operator/=(tvec2<T>& l, const tvec2<U>& r) { l.x /= r.x; l.y /= r.y; return l; }
template<class T> inline tvec2<T>& operator+=(tvec2<T>& l, T r) { l.x += r; l.y += r; return l; }
template<class T> inline tvec2<T>& operator-=(tvec2<T>& l, T r) { l.x -= r; l.y -= r; return l; }
template<class T> inline tvec2<T>& operator*=(tvec2<T>& l, T r) { l.x *= r; l.y *= r; return l; }
template<class T> inline tvec2<T>& operator/=(tvec2<T>& l, T r) { l.x /= r; l.y /= r; return l; }

template<class T> inline tvec3<T> operator-(const tvec3<T>& v) { return{ -v.x, -v.y, -v.z }; }
template<class T, class U> inline tvec3<T> operator+(const tvec3<T>& l, const tvec3<U>& r) { return{ l.x + r.x, l.y + r.y, l.z + r.z }; }
template<class T, class U> inline tvec3<T> operator-(const tvec3<T>& l, const tvec3<U>& r) { return{ l.x - r.x, l.y - r.y, l.z - r.z }; }
template<class T, class U> inline tvec3<T> operator*(const tvec3<T>& l, const tvec3<U>& r) { return{ l.x * r.x, l.y * r.y, l.z * r.z }; }
template<class T, class U> inline tvec3<T> operator/(const tvec3<T>& l, const tvec3<U>& r) { return{ l.x / r.x, l.y / r.y, l.z / r.z }; }
template<class T> inline tvec3<T> operator+(T l, const tvec3<T>& r) { return{ l + r.x, l + r.y, l + r.z }; }
template<class T> inline tvec3<T> operator-(T l, const tvec3<T>& r) { return{ l - r.x, l - r.y, l - r.z }; }
template<class T> inline tvec3<T> operator*(T l, const tvec3<T>& r) { return{ l * r.x, l * r.y, l * r.z }; }
template<class T> inline tvec3<T> operator/(T l, const tvec3<T>& r) { return{ l / r.x, l / r.y, l / r.z }; }
template<class T> inline tvec3<T> operator+(const tvec3<T>& l, T r) { return{ l.x + r, l.y + r, l.z + r }; }
template<class T> inline tvec3<T> operator-(const tvec3<T>& l, T r) { return{ l.x - r, l.y - r, l.z - r }; }
template<class T> inline tvec3<T> operator*(const tvec3<T>& l, T r) { return{ l.x * r, l.y * r, l.z * r }; }
template<class T> inline tvec3<T> operator/(const tvec3<T>& l, T r) { return{ l.x / r, l.y / r, l.z / r }; }
template<class T, class U> inline tvec3<T>& operator+=(tvec3<T>& l, const tvec3<U>& r) { l.x += r.x; l.y += r.y; l.z += r.z; return l; }
template<class T, class U> inline tvec3<T>& operator-=(tvec3<T>& l, const tvec3<U>& r) { l.x -= r.x; l.y -= r.y; l.z -= r.z; return l; }
template<class T, class U> inline tvec3<T>& operator*=(tvec3<T>& l, const tvec3<U>& r) { l.x *= r.x; l.y *= r.y; l.z *= r.z; return l; }
template<class T, class U> inline tvec3<T>& operator/=(tvec3<T>& l, const tvec3<U>& r) { l.x /= r.x; l.y /= r.y; l.z /= r.z; return l; }
template<class T> inline tvec3<T>& operator+=(tvec3<T>& l, T r) { l.x += r; l.y += r; l.z += r; return l; }
template<class T> inline tvec3<T>& operator-=(tvec3<T>& l, T r) { l.x -= r; l.y -= r; l.z -= r; return l; }
template<class T> inline tvec3<T>& operator*=(tvec3<T>& l, T r) { l.x *= r; l.y *= r; l.z *= r; return l; }
template<class T> inline tvec3<T>& operator/=(tvec3<T>& l, T r) { l.x /= r; l.y /= r; l.z /= r; return l; }

template<class T> inline tvec4<T> operator-(const tvec4<T>& v) { return{ -v.x, -v.y, -v.z, -v.w }; }
template<class T, class U> inline tvec4<T> operator+(const tvec4<T>& l, const tvec4<U>& r) { return{ l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w }; }
template<class T, class U> inline tvec4<T> operator-(const tvec4<T>& l, const tvec4<U>& r) { return{ l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w }; }
template<class T, class U> inline tvec4<T> operator*(const tvec4<T>& l, const tvec4<U>& r) { return{ l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w }; }
template<class T, class U> inline tvec4<T> operator/(const tvec4<T>& l, const tvec4<U>& r) { return{ l.x / r.x, l.y / r.y, l.z / r.z, l.w / r.w }; }
template<class T> inline tvec4<T> operator+(T l, const tvec4<T>& r) { return{ l + r.x, l + r.y, l + r.z, l + r.w }; }
template<class T> inline tvec4<T> operator-(T l, const tvec4<T>& r) { return{ l - r.x, l - r.y, l - r.z, l - r.w }; }
template<class T> inline tvec4<T> operator*(T l, const tvec4<T>& r) { return{ l * r.x, l * r.y, l * r.z, l * r.w }; }
template<class T> inline tvec4<T> operator/(T l, const tvec4<T>& r) { return{ l / r.x, l / r.y, l / r.z, l / r.w }; }
template<class T> inline tvec4<T> operator+(const tvec4<T>& l, T r) { return{ l.x + r, l.y + r, l.z + r, l.w + r }; }
template<class T> inline tvec4<T> operator-(const tvec4<T>& l, T r) { return{ l.x - r, l.y - r, l.z - r, l.w - r }; }
template<class T> inline tvec4<T> operator*(const tvec4<T>& l, T r) { return{ l.x * r, l.y * r, l.z * r, l.w * r }; }
template<class T> inline tvec4<T> operator/(const tvec4<T>& l, T r) { return{ l.x / r, l.y / r, l.z / r, l.w / r }; }
template<class T, class U> inline tvec4<T>& operator+=(tvec4<T>& l, const tvec4<U>& r) { l.x += r.x; l.y += r.y; l.z += r.z; l.w += r.w; return l; }
template<class T, class U> inline tvec4<T>& operator-=(tvec4<T>& l, const tvec4<U>& r) { l.x -= r.x; l.y -= r.y; l.z -= r.z; l.w -= r.w; return l; }
template<class T, class U> inline tvec4<T>& operator*=(tvec4<T>& l, const tvec4<U>& r) { l.x *= r.x; l.y *= r.y; l.z *= r.z; l.w *= r.w; return l; }
template<class T, class U> inline tvec4<T>& operator/=(tvec4<T>& l, const tvec4<U>& r) { l.x /= r.x; l.y /= r.y; l.z /= r.z; l.w /= r.w; return l; }
template<class T> inline tvec4<T>& operator+=(tvec4<T>& l, T r) { l.x += r; l.y += r; l.z += r; l.w += r; return l; }
template<class T> inline tvec4<T>& operator-=(tvec4<T>& l, T r) { l.x -= r; l.y -= r; l.z -= r; l.w -= r; return l; }
template<class T> inline tvec4<T>& operator*=(tvec4<T>& l, T r) { l.x *= r; l.y *= r; l.z *= r; l.w *= r; return l; }
template<class T> inline tvec4<T>& operator/=(tvec4<T>& l, T r) { l.x /= r; l.y /= r; l.z /= r; l.w /= r; return l; }


template<class T> inline tquat<T> operator*(const tquat<T>& l, T r) { return{ l.x*r, l.y*r, l.z*r, l.w*r }; }
template<class T> inline tquat<T> operator*(const tquat<T>& l, const tquat<T>& r)
{
    return{
        l.w*r.x + l.x*r.w + l.y*r.z - l.z*r.y,
        l.w*r.y + l.y*r.w + l.z*r.x - l.x*r.z,
        l.w*r.z + l.z*r.w + l.x*r.y - l.y*r.x,
        l.w*r.w - l.x*r.x - l.y*r.y - l.z*r.z,
    };
}
template<class T> inline tquat<T>& operator*=(tquat<T>& l, T r)
{
    l = l * r;
    return l;
}
template<class T> inline tquat<T>& operator*=(tquat<T>& l, const tquat<T>& r)
{
    l = l * r;
    return l;
}

template<class T> inline tvec3<T> operator*(const tmat3x3<T>& m, const tvec3<T>& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
template<class T> inline tvec3<T> operator*(const tmat4x4<T>& m, const tvec3<T>& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
template<class T> inline tvec4<T> operator*(const tmat4x4<T>& m, const tvec4<T>& v)
{
    return{
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3],
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3],
    };
}
template<class T> inline tmat4x4<T> operator*(const tmat4x4<T> &a, const tmat4x4<T> &b)
{
    tmat4x4<T> c;
    const T *ap = &a[0][0];
    const T *bp = &b[0][0];
    T *cp = &c[0][0];
    T a0, a1, a2, a3;

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
template<class T> inline tmat4x4<T>& operator*=(tmat4x4<T>& a, const tmat4x4<T> &b)
{
    a = a * b;
    return a;
}

inline int ceildiv(int v, int d) { return (v + (d - 1)) / d; }

#define SF(T)                                                                                       \
    inline T rcp(T v) { return T(1.0) / v; }                                                        \
    inline T rsqrt(T v) { return T(1.0) / std::sqrt(v); }                                           \
    inline T mod(T a, T b) { return std::fmod(a, b); }                                              \
    inline T frac(T a) { return std::fmod(a, T(1.0)); }                                             \
    inline T clamp(T v, T vmin, T vmax) { return std::min<T>(std::max<T>(v, vmin), vmax); }         \
    inline T clamp01(T v) { return clamp(v, T(0), T(1)); }                                          \
    inline T saturate(T v) { return clamp(v, T(-1), T(1)); }                                        \
    inline T lerp(T  a, T  b, T  t) { return a * (T(1.0) - t) + b * t; }                            \
    inline bool near_equal(T a, T b, T epsilon = T(muEpsilon)) { return std::abs(a - b) < epsilon; }\

SF(float)
SF(double)
#ifdef muEnableHalf
SF(half)
#endif
#undef SF

#define VF1N(N, F)\
    template<class T> inline tvec2<T> N(const tvec2<T>& a) { return{ F(a.x), F(a.y) }; }\
    template<class T> inline tvec3<T> N(const tvec3<T>& a) { return{ F(a.x), F(a.y), F(a.z) }; }\
    template<class T> inline tvec4<T> N(const tvec4<T>& a) { return{ F(a.x), F(a.y), F(a.z), F(a.w) }; }\

#define VF2N(N, F)\
    template<class T> inline tvec2<T> N(const tvec2<T>& a, const tvec2<T>& b) { return{ F(a.x, b.x), F(a.y, b.y) }; }\
    template<class T> inline tvec3<T> N(const tvec3<T>& a, const tvec3<T>& b) { return{ F(a.x, b.x), F(a.y, b.y), F(a.z, b.z) }; }\
    template<class T> inline tvec4<T> N(const tvec4<T>& a, const tvec4<T>& b) { return{ F(a.x, b.x), F(a.y, b.y), F(a.z, b.z), F(a.w, b.w) }; }\

#define VF1(N) VF1N(N, N)
#define VF2(N) VF2N(N, N)
#define VF1std(N) using std::N; VF1N(N, N)
#define VF2std(N) using std::N; VF2N(N, N)

VF1std(abs)
VF1std(round)
VF1std(floor)
VF1std(ceil)
VF2std(min)
VF2std(max)
VF1(rcp)
VF1std(sqrt)
VF1(rsqrt)
VF1std(sin)
VF1std(cos)
VF1std(tan)
VF1std(asin)
VF1std(acos)
VF1std(atan)
VF2std(atan2)
VF1std(exp)
VF1std(log)
VF2std(pow)
VF2(mod)
VF1(frac)
VF1(clamp01)
VF1(saturate)

#undef VF1N
#undef VF2N
#undef VF1
#undef VF2
#undef VF1std
#undef VF2std

template<class T> inline tvec2<T> clamp(const tvec2<T>& v, const tvec2<T>& vmin, const tvec2<T>& vmax) { return { clamp<T>(v.x, vmin.x, vmax.x), clamp<T>(v.y, vmin.y, vmax.y) }; }
template<class T> inline tvec3<T> clamp(const tvec3<T>& v, const tvec3<T>& vmin, const tvec3<T>& vmax) { return { clamp<T>(v.x, vmin.x, vmax.x), clamp<T>(v.y, vmin.y, vmax.y), clamp<T>(v.z, vmin.z, vmax.z) }; }
template<class T> inline tvec4<T> clamp(const tvec4<T>& v, const tvec4<T>& vmin, const tvec4<T>& vmax) { return { clamp<T>(v.x, vmin.x, vmax.x), clamp<T>(v.y, vmin.y, vmax.y), clamp<T>(v.z, vmin.z, vmax.z), clamp<T>(v.w, vmin.w, vmax.w) }; }

template<class T> inline tvec2<T> lerp(const tvec2<T>& a, const tvec2<T>& b, T t) { return a*(T(1.0) - t) + b*t; }
template<class T> inline tvec3<T> lerp(const tvec3<T>& a, const tvec3<T>& b, T t) { return a*(T(1.0) - t) + b*t; }
template<class T> inline tvec4<T> lerp(const tvec4<T>& a, const tvec4<T>& b, T t) { return a*(T(1.0) - t) + b*t; }

template<class T> inline bool near_equal(const tvec2<T>& a, const tvec2<T>& b, T e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e);
}
template<class T> inline bool near_equal(const tvec3<T>& a, const tvec3<T>& b, T e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e);
}
template<class T> inline bool near_equal(const tvec4<T>& a, const tvec4<T>& b, T e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e) && near_equal(a.w, b.w, e);
}
template<class T> inline bool near_equal(const tquat<T>& a, const tquat<T>& b, T e = muEpsilon)
{
    return near_equal(a.x, b.x, e) && near_equal(a.y, b.y, e) && near_equal(a.z, b.z, e) && near_equal(a.w, b.w, e);
}
template<class T> inline bool near_equal(const tmat3x3<T>& a, const tmat3x3<T>& b, T e = muEpsilon)
{
    return near_equal(a[0], b[0], e) && near_equal(a[1], b[1], e) && near_equal(a[2], b[2], e);
}
template<class T> inline bool near_equal(const tmat4x4<T>& a, const tmat4x4<T>& b, T e = muEpsilon)
{
    return near_equal(a[0], b[0], e) && near_equal(a[1], b[1], e) && near_equal(a[2], b[2], e) && near_equal(a[3], b[3], e);
}

template<class T>
inline static tvec3<T> mul_v(const tmat4x4<T>& m, const tvec3<T>& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
    };
}
template<class T>
inline static tvec4<T> mul_v(const tmat4x4<T>& m, const tvec4<T>& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2],
        v[3],
    };
}
template<class T>
inline static tvec3<T> mul_p(const tmat4x4<T>& m, const tvec3<T>& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2],
    };
}
template<class T>
inline static tvec4<T> mul4(const tmat4x4<T>& m, const tvec3<T>& v)
{
    return {
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2],
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3],
    };
}

template<class T> inline T dot(const tvec2<T>& l, const tvec2<T>& r) { return l.x*r.x + l.y*r.y; }
template<class T> inline T dot(const tvec3<T>& l, const tvec3<T>& r) { return l.x*r.x + l.y*r.y + l.z*r.z; }
template<class T> inline T length_sq(const tvec2<T>& v) { return dot(v, v); }
template<class T> inline T length_sq(const tvec3<T>& v) { return dot(v, v); }
template<class T> inline T length(const tvec2<T>& v) { return sqrt(length_sq(v)); }
template<class T> inline T length(const tvec3<T>& v) { return sqrt(length_sq(v)); }
template<class T> inline tvec3<T> normalize(const tvec3<T>& v) { return v / length(v); }
template<class T> inline tvec3<T> cross(const tvec3<T>& l, const tvec3<T>& r)
{
    return{
        l.y * r.z - l.z * r.y,
        l.z * r.x - l.x * r.z,
        l.x * r.y - l.y * r.x };
}

// a & b must be normalized
template<class T> inline T angle_between(const tvec3<T>& a, const tvec3<T>& b)
{
    return std::acos(dot(a, b));
}
template<class T> inline T angle_between2(const tvec3<T>& pos1, const tvec3<T>& pos2, const tvec3<T>& center)
{
    return angle_between(
        normalize(pos1 - center),
        normalize(pos2 - center));
}

template<class T> inline tvec3<T> apply_rotation(const tquat<T>& q, const tvec3<T>& p)
{
    auto a = cross(reinterpret_cast<tvec3<T>&>(q), p);
    auto b = cross(reinterpret_cast<tvec3<T>&>(q), a);
    return p + (a * q.w + b) * T(2.0);
}

template<class T> inline tquat<T> invert(const tquat<T>& v)
{
    return{ -v.x, -v.y, -v.z, v.w };
}

template<class T> inline tquat<T> flipY(const tquat<T>& v)
{
    return{ -v.z, v.w, v.x, -v.y, };
}

template<class T> inline tquat<T> rotateX(T angle)
{
    T c = std::cos(angle * T(0.5));
    T s = std::sin(angle * T(0.5));
    return{ s, T(0.0), T(0.0), c };
}
template<class T> inline tquat<T> rotateY(T angle)
{
    T c = std::cos(angle * T(0.5));
    T s = std::sin(angle * T(0.5));
    return{ T(0.0), s, T(0.0), c };
}
template<class T> inline tquat<T> rotateZ(T angle)
{
    T c = std::cos(angle * T(0.5));
    T s = std::sin(angle * T(0.5));
    return{ T(0.0), T(0.0), s, c };
}

// eular -> quaternion
template<class T> inline tquat<T> rotateXYZ(const tvec3<T>& euler)
{
    auto qX = rotateX(euler.x);
    auto qY = rotateY(euler.y);
    auto qZ = rotateZ(euler.z);
    return (qZ * qY) * qX;
}
template<class T> inline tquat<T> rotateXZY(const tvec3<T>& euler)
{
    auto qX = rotateX(euler.x);
    auto qY = rotateY(euler.y);
    auto qZ = rotateZ(euler.z);
    return (qY * qZ) * qX;
}
template<class T> inline tquat<T> rotateYXZ(const tvec3<T>& euler)
{
    auto qX = rotateX(euler.x);
    auto qY = rotateY(euler.y);
    auto qZ = rotateZ(euler.z);
    return (qZ * qX) * qY;
}
template<class T> inline tquat<T> rotateYZX(const tvec3<T>& euler)
{
    auto qX = rotateX(euler.x);
    auto qY = rotateY(euler.y);
    auto qZ = rotateZ(euler.z);
    return (qX * qZ) * qY;
}
template<class T> inline tquat<T> rotateZXY(const tvec3<T>& euler)
{
    auto qX = rotateX(euler.x);
    auto qY = rotateY(euler.y);
    auto qZ = rotateZ(euler.z);
    return (qY * qX) * qZ;
}
template<class T> inline tquat<T> rotateZYX(const tvec3<T>& euler)
{
    auto qX = rotateX(euler.x);
    auto qY = rotateY(euler.y);
    auto qZ = rotateZ(euler.z);
    return (qX * qY) * qZ;
}

template<class T> inline tquat<T> rotate(const tvec3<T>& axis, T angle)
{
    return{
        axis.x * std::sin(angle * T(0.5)),
        axis.y * std::sin(angle * T(0.5)),
        axis.z * std::sin(angle * T(0.5)),
        std::cos(angle * T(0.5))
    };
}

template<class T> inline tvec3<T> to_eularZXY(const tquat<T>& q)
{
    T d[] = {
        q.x*q.x, q.x*q.y, q.x*q.z, q.x*q.w,
        q.y*q.y, q.y*q.z, q.y*q.w,
        q.z*q.z, q.z*q.w,
        q.w*q.w
    };

    T v0 = d[5] - d[3];
    T v1 = T(2.0) * (d[1] + d[8]);
    T v2 = d[4] - d[7] - d[0] + d[9];
    T v3 = T(-1.0);
    T v4 = T(2.0) * v0;

    if (std::abs(v0) < T(0.499999))
    {
        T v5 = T(2.0) * (d[2] + d[6]);
        T v6 = d[7] - d[0] - d[4] + d[9];

        return{
            v3 * std::asin(saturate(v4)),
            std::atan2(v5, v6),
            std::atan2(v1, v2)
        };
    }
    else
    {
        T a = d[1] + d[8];
        T b =-d[5] + d[3];
        T c = d[1] - d[8];
        T e = d[5] + d[3];
        T v5 = a*e + b*c;
        T v6 = b*e - a*c;
        return{
            v3 * std::asin(saturate(v4)),
            std::atan2(v5, v6),
            T(0.0)
        };
    }
}

template<class T> inline void to_axis_angle(const tquat<T>& q, tvec3<T>& axis, T& angle)
{
    angle = T(2.0) * std::acos(q.w);
    axis = {
        q.x / std::sqrt(T(1.0) - q.w*q.w),
        q.y / std::sqrt(T(1.0) - q.w*q.w),
        q.z / std::sqrt(T(1.0) - q.w*q.w)
    };
}

template<class T> inline tmat3x3<T> look33(const tvec3<T>& dir, const tvec3<T>& up)
{
    auto z = dir;
    auto x = normalize(cross(up, z));
    auto y = cross(z, x);
    return{ {
        { x.x, y.x, z.x },
        { x.y, y.y, z.y },
        { x.z, y.z, z.z },
    } };
}

template<class T> inline tvec3<T> swap_handedness(const tvec3<T>& v) { return { -v.x, v.y, v.z }; }
template<class T> inline tvec4<T> swap_handedness(const tvec4<T>& v) { return { -v.x, v.y, v.z, v.w }; }
template<class T> inline tquat<T> swap_handedness(const tquat<T>& v) { return { v.x, -v.y, -v.z, v.w }; }
template<class T> inline tmat3x3<T> swap_handedness(const tmat3x3<T>& m)
{
    return{ {
        { m[0].x,-m[0].y,-m[0].z },
        {-m[1].x, m[1].y, m[1].z },
        {-m[2].x, m[2].y, m[2].z },
    } };
}
template<class T> inline tmat4x4<T> swap_handedness(const tmat4x4<T>& m)
{
    return{ {
        { m[0].x,-m[0].y,-m[0].z, m[0].w },
        {-m[1].x, m[1].y, m[1].z, m[1].w },
        {-m[2].x, m[2].y, m[2].z, m[2].w },
        {-m[3].x, m[3].y, m[3].z, m[3].w },
    } };
}

template<class T> inline tmat3x3<T> to_mat3x3(const tquat<T>& q)
{
    return {{
        {T(1.0)-T(2.0)*q.y*q.y - T(2.0)*q.z*q.z,T(2.0)*q.x*q.y - T(2.0)*q.z*q.w,         T(2.0)*q.x*q.z + T(2.0)*q.y*q.w,        },
        {T(2.0)*q.x*q.y + T(2.0)*q.z*q.w,       T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.z*q.z,T(2.0)*q.y*q.z - T(2.0)*q.x*q.w,        },
        {T(2.0)*q.x*q.z - T(2.0)*q.y*q.w,       T(2.0)*q.y*q.z + T(2.0)*q.x*q.w,         T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.y*q.y}
    }};
}
template<class T> inline tmat4x4<T> to_mat4x4(const tquat<T>& q)
{
    return {{
        {T(1.0)-T(2.0)*q.y*q.y - T(2.0)*q.z*q.z,T(2.0)*q.x*q.y - T(2.0)*q.z*q.w,         T(2.0)*q.x*q.z + T(2.0)*q.y*q.w,         T(0.0)},
        {T(2.0)*q.x*q.y + T(2.0)*q.z*q.w,       T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.z*q.z,T(2.0)*q.y*q.z - T(2.0)*q.x*q.w,         T(0.0)},
        {T(2.0)*q.x*q.z - T(2.0)*q.y*q.w,       T(2.0)*q.y*q.z + T(2.0)*q.x*q.w,         T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.y*q.y,T(0.0)},
        {T(0.0),                                T(0.0),                                  T(0.0),                                  T(1.0)}
    }};
}

template<class T> inline tmat4x4<T> translate(const tvec3<T>& t)
{
    return {{
        { T(1.0), T(0.0), T(0.0), T(0.0) },
        { T(0.0), T(1.0), T(0.0), T(0.0) },
        { T(0.0), T(0.0), T(1.0), T(0.0) },
        {    t.x,    t.y,    t.z, T(1.0) }
    }};
}

template<class T> inline tmat3x3<T> scale33(const tvec3<T>& t)
{
    return{{
        {    t.x, T(0.0), T(0.0) },
        { T(0.0),    t.y, T(0.0) },
        { T(0.0), T(0.0),    t.z },
    }};
}
template<class T> inline tmat4x4<T> scale44(const tvec3<T>& t)
{
    return{{
        {    t.x, T(0.0), T(0.0), T(0.0) },
        { T(0.0),    t.y, T(0.0), T(0.0) },
        { T(0.0), T(0.0),    t.z, T(0.0) },
        { T(0.0), T(0.0), T(0.0), T(1.0) }
    }};
}

template<class T> inline tmat4x4<T> transform(const tvec3<T>& t, const tquat<T>& r, const tvec3<T>& s)
{
    auto ret = scale44(s);
    ret *= to_mat4x4(r);
    ret *= translate(t);
    return ret;
}

template<class T> inline tmat3x3<T> invert(const tmat3x3<T>& x)
{
    if (x[0][2] != 0 || x[1][2] != 0 || x[2][2] != 1) {
        tmat3x3<T> s = {
            x[1][1] * x[2][2] - x[2][1] * x[1][2],
            x[2][1] * x[0][2] - x[0][1] * x[2][2],
            x[0][1] * x[1][2] - x[1][1] * x[0][2],

            x[2][0] * x[1][2] - x[1][0] * x[2][2],
            x[0][0] * x[2][2] - x[2][0] * x[0][2],
            x[1][0] * x[0][2] - x[0][0] * x[1][2],

            x[1][0] * x[2][1] - x[2][0] * x[1][1],
            x[2][0] * x[0][1] - x[0][0] * x[2][1],
            x[0][0] * x[1][1] - x[1][0] * x[0][1] };

        T r = x[0][0] * s[0][0] + x[0][1] * s[1][0] + x[0][2] * s[2][0];

        if (std::abs(r) >= T(1.0)) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    s[i][j] /= r;
                }
            }
        }
        else {
            T mr = std::abs(r) / std::numeric_limits<T>::min();

            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    if (mr > std::abs(s[i][j])) {
                        s[i][j] /= r;
                    }
                    else {
                        // singular
                        return tmat3x3<T>::identity();
                    }
                }
            }
        }

        return s;
    }
    else {
        tmat3x3<T> s = {
             x[1][1], -x[0][1], 0,
            -x[1][0],  x[0][0], 0,
                   0,        0, 1 };

        T r = x[0][0] * x[1][1] - x[1][0] * x[0][1];

        if (std::abs(r) >= 1) {
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    s[i][j] /= r;
                }
            }
        }
        else {
            T mr = std::abs(r) / std::numeric_limits<T>::min();

            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    if (mr > std::abs(s[i][j])) {
                        s[i][j] /= r;
                    }
                    else
                    {
                        // singular
                        return tmat3x3<T>::identity();
                    }
                }
            }
        }

        s[2][0] = -x[2][0] * s[0][0] - x[2][1] * s[1][0];
        s[2][1] = -x[2][0] * s[0][1] - x[2][1] * s[1][1];
        return s;
    }
}

template<class T> inline tmat4x4<T> invert(const tmat4x4<T>& x)
{
    tmat4x4<T> s = {
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

        0, 0, 0, 1,
    };

    auto r = x[0][0] * s[0][0] + x[0][1] * s[1][0] + x[0][2] * s[2][0];

    if (std::abs(r) >= 1) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                s[i][j] /= r;
            }
        }
    }
    else {
        auto mr = std::abs(r) / std::numeric_limits<T>::min();

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (mr > std::abs(s[i][j])) {
                    s[i][j] /= r;
                }
                else {
                    // error
                    return tmat4x4<T>::identity();
                }
            }
        }
    }

    s[3][0] = -x[3][0] * s[0][0] - x[3][1] * s[1][0] - x[3][2] * s[2][0];
    s[3][1] = -x[3][0] * s[0][1] - x[3][1] * s[1][1] - x[3][2] * s[2][1];
    s[3][2] = -x[3][0] * s[0][2] - x[3][1] * s[1][2] - x[3][2] * s[2][2];
    return s;
}

template<class TMat>
inline tquat<typename TMat::scalar_t> to_quat_impl(const TMat& m)
{
    using T = typename TMat::scalar_t;
    T tr, s;
    T q[4];
    int i, j, k;
    tquat<T> quat;

    int nxt[3] = { 1, 2, 0 };
    tr = m[0][0] + m[1][1] + m[2][2];

    // check the diagonal
    if (tr > 0.0) {
        s = std::sqrt(tr + T(1.0));
        quat.w = s / T(2.0);
        s = T(0.5) / s;

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
        s = std::sqrt((m[i][i] - (m[j][j] + m[k][k])) + T(1.0));

        q[i] = s * T(0.5);
        if (s != T(0.0))
            s = T(0.5) / s;

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

template<class T> inline quatf to_quat(const tmat3x3<T>& m)
{
    return to_quat_impl(m);
}
template<class T> inline quatf to_quat(const tmat4x4<T>& m)
{
    return to_quat_impl(m);
}

// aperture and focal_length must be millimeter. return fov in degree
template<class T> inline T compute_fov(T aperture, T focal_length)
{
    return T(2.0) * std::atan(aperture / (T(2.0) * focal_length)) * Rad2Deg;
}

// aperture: millimeter
// fov: degree
template<class T> inline T compute_focal_length(T aperture, T fov)
{
    return aperture / std::tan(fov * Deg2Rad / T(2.0)) / T(2.0);
}


template<class T>
inline bool ray_triangle_intersection(
    const tvec3<T>& pos, const tvec3<T>& dir, const tvec3<T>& p1, const tvec3<T>& p2, const tvec3<T>& p3, T& distance)
{
    const T epsdet = 1e-10f;
    const T eps = 1e-4f;

    auto e1 = p2 - p1;
    auto e2 = p3 - p1;
    auto p = cross(dir, e2);
    auto det = dot(e1, p);
    if (std::abs(det) < epsdet) return false;
    auto inv_det = T(1.0) / det;
    auto t = pos - p1;
    auto u = dot(t, p) * inv_det;
    if (u < -eps || u  > 1 + eps) return false;
    auto q = cross(t, e1);
    auto v = dot(dir, q) * inv_det;
    if (v < -eps || u + v > 1 + eps) return false;

    distance = dot(e2, q) * inv_det;
    return distance >= T(0.0);
}

// pos must be on the triangle
template<class T, class U>
inline U triangle_interpolation(
    const tvec3<T>& pos, const tvec3<T>& p1, const tvec3<T>& p2, const tvec3<T>& p3, U x1, U x2, U x3)
{
    auto f1 = p1 - pos;
    auto f2 = p2 - pos;
    auto f3 = p3 - pos;
    auto a = T(1.0) / length(cross(p1 - p2, p1 - p3));
    auto a1 = length(cross(f2, f3)) * a;
    auto a2 = length(cross(f3, f1)) * a;
    auto a3 = length(cross(f1, f2)) * a;
    return x1 * a1 + x2 * a2 + x3 * a3;
}

template<class T> inline T ray_point_distance(const tvec3<T>& pos, const tvec3<T>& dir, const tvec3<T>& p)
{
    return length(cross(dir, p - pos));
}

template<class T> inline T plane_distance(const tvec3<T>& p, const tvec3<T>& pn, T pd) { return dot(p, pn) - pd; }
template<class T> inline T plane_distance(const tvec3<T>& p, const tvec3<T>& pn)       { return dot(p, pn); }
template<class T> inline tvec3<T> plane_mirror(const tvec3<T>& p, const tvec3<T>& pn, T pd) { return p - pn * (plane_distance(p, pn, pd) * T(2.0)); }
template<class T> inline tvec3<T> plane_mirror(const tvec3<T>& p, const tvec3<T>& pn)       { return p - pn * (plane_distance(p, pn) * T(2.0)); }

template<class T> inline void compute_triangle_tangent(
    const tvec3<T>(&vertices)[3], const tvec2<T>(&uv)[3], tvec3<T>(&dst_tangent)[3], tvec3<T>(&dst_binormal)[3])
{
    auto p = vertices[1] - vertices[0];
    auto q = vertices[2] - vertices[0];
    auto s = tvec2<T>{ uv[1].x - uv[0].x, uv[2].x - uv[0].x };
    auto t = tvec2<T>{ uv[1].y - uv[0].y, uv[2].y - uv[0].y };

    T div = s.x * t.y - s.y * t.x;
    T area = std::abs(div);
    T rdiv = T(1.0) / div;
    s *= rdiv;
    t *= rdiv;

    auto tangent = normalize(tvec3<T>{
        t.y * p.x - t.x * q.x,
        t.y * p.y - t.x * q.y,
        t.y * p.z - t.x * q.z
    }) * area;
    auto binormal = normalize(tvec3<T>{
        s.x * q.x - s.y * p.x,
        s.x * q.y - s.y * p.y,
        s.x * q.z - s.y * p.z
    }) * area;

    T angles[3] = {
        angle_between2(vertices[2], vertices[1], vertices[0]),
        angle_between2(vertices[0], vertices[2], vertices[1]),
        angle_between2(vertices[1], vertices[0], vertices[2]),
    };
    for (int v = 0; v < 3; ++v)
    {
        dst_tangent[v] = tangent * angles[v];
        dst_binormal[v] = binormal * angles[v];
    }
}

template<class T> inline tvec4<T> orthogonalize_tangent(
    tvec3<T> tangent, tvec3<T> binormal, tvec3<T> normal)
{
    auto NdotT = dot(normal, tangent);
    tangent -= normal * NdotT;
    auto magT = length(tangent);
    tangent = tangent / magT;

    auto NdotB = dot(normal, binormal);
    auto TdotB = dot(tangent, binormal) * magT;
    binormal -= normal * NdotB - tangent * TdotB;;
    auto magB = length(binormal);
    binormal = binormal / magB;

#if 0
    const auto epsilon = 1e-6f;
    if (magT <= epsilon || magB <= epsilon)
    {
        tvec3<T> axis1, axis2;

        auto dpXN = std::abs(dot({ T(1.0), T(0.0), T(0.0) }, normal));
        auto dpYN = std::abs(dot({ T(0.0), T(1.0), T(0.0) }, normal));
        auto dpZN = std::abs(dot({ T(0.0), T(0.0), T(1.0) }, normal));

        if (dpXN <= dpYN && dpXN <= dpZN)
        {
            axis1 = { T(1.0), T(0.0), T(0.0) };
            if (dpYN <= dpZN)
                axis2 = { T(0.0), T(1.0), T(0.0) };
            else
                axis2 = { T(0.0), T(0.0), T(1.0) };
        }
        else if (dpYN <= dpXN && dpYN <= dpZN)
        {
            axis1 = { T(0.0), T(1.0), T(0.0) };
            if (dpXN <= dpZN)
                axis2 = { T(1.0), T(0.0), T(0.0) };
            else
                axis2 = { T(0.0), T(0.0), T(1.0) };
        }
        else
        {
            axis1 = { T(0.0), T(0.0), T(1.0) };
            if (dpXN <= dpYN)
                axis2 = { T(1.0), T(0.0), T(0.0) };
            else
                axis2 = { T(0.0), T(1.0), T(0.0) };
        }
        tangent = normalize(axis1 - normal * dot(normal, axis1));
        binormal = normalize(axis2 - normal * dot(normal, axis2) - normalize(tangent) * dot(tangent, axis2));
    }
#endif

    return { tangent.x, tangent.y, tangent.z,
        dot(cross(normal, tangent), binormal) > T(0.0) ? T(1.0) : -T(1.0) };
}

} // namespace mu
