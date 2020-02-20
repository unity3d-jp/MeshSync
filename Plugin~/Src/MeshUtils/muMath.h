#pragma once

#include <cmath>
#include <cstring>
#include <algorithm>
#include <limits>
#include "muIntrusiveArray.h"
#include "muHalf.h"

#define muEpsilon 1e-4f

namespace mu {

constexpr double PI_d = 3.14159265358979323846264338327950288419716939937510;
constexpr double DegToRad_d = PI_d / 180.0;
constexpr double RadToDeg_d = 1.0f / (PI_d / 180.0);
constexpr double InchToMillimeter_d = 25.4;

constexpr float PI = 3.14159265358979323846264338327950288419716939937510f;
constexpr float DegToRad = PI / 180.0f;
constexpr float RadToDeg = 1.0f / (PI / 180.0f);
constexpr float InchToMillimeter = 25.4f;


template<class T>
struct tvec2
{
    using scalar_t = T;
    static const int vector_length = 2;

    T x, y;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const tvec2& v) const { return !((*this)==v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1] }; }
    template<class U> void assign(const tvec2<U>& v) { assign((const U*)&v); }

    static constexpr tvec2 zero() { return{ (T)0, (T)0 }; }
    static constexpr tvec2 one() { return{ (T)1, (T)1 }; }
    static constexpr tvec2 set(T v) { return{ v, v }; }
};

template<class T>
struct tvec3
{
    using scalar_t = T;
    static const int vector_length = 3;

    T x, y, z;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const tvec3& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1], (T)v[2] }; }
    template<class U> void assign(const tvec3<U>& v) { assign((const U*)&v); }

    static constexpr tvec3 zero() { return{ (T)0, (T)0, (T)0 }; }
    static constexpr tvec3 one() { return{ (T)1, (T)1, (T)1 }; }
    static constexpr tvec3 set(T v) { return{ v, v, v }; }
};

template<class T>
struct tvec4
{
    using scalar_t = T;
    static const int vector_length = 4;

    T x, y, z, w;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tvec4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const tvec4& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1], (T)v[2], (T)v[3] }; }
    template<class U> void assign(const tvec4<U>& v) { assign((const U*)&v); }

    static constexpr tvec4 zero() { return{ (T)0, (T)0, (T)0, (T)0 }; }
    static constexpr tvec4 one() { return{ (T)1, (T)1, (T)1, (T)1 }; }
    static constexpr tvec4 set(T v) { return{ v, v, v, v }; }
};

template<class T>
struct tquat
{
    using scalar_t = T;
    static const int vector_length = 4;

    T x, y, z, w;
    T& operator[](int i) { return ((T*)this)[i]; }
    const T& operator[](int i) const { return ((T*)this)[i]; }
    bool operator==(const tquat& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const tquat& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v) { *this = { (T)v[0], (T)v[1], (T)v[2], (T)v[3] }; }
    template<class U> void assign(const tquat<U>& v) { assign((const U*)&v); }

    static constexpr tquat identity() { return{ (T)0, (T)0, (T)0, (T)1 }; }
};

template<class T>
struct tmat2x2
{
    using scalar_t = T;
    using vector_t = tvec2<T>;
    static const int vector_length = 4;

    tvec2<T> m[2];
    tvec2<T>& operator[](int i) { return m[i]; }
    const tvec2<T>& operator[](int i) const { return m[i]; }
    bool operator==(const tmat2x2& v) const { return memcmp(m, v.m, sizeof(*this)) == 0; }
    bool operator!=(const tmat2x2& v) const { return !((*this) == v); }

    template<class U> void assign(const U *v)
    {
        *this = { {
            { (T)v[0], (T)v[1] },
            { (T)v[2], (T)v[3] },
        } };
    }
    template<class U> void assign(const tmat2x2<U>& v) { assign((U*)&v); }

    static constexpr tmat2x2 identity()
    {
        return{ {
            { T(1.0), T(0.0) },
            { T(0.0), T(1.0) },
        } };
    }
};

template<class T>
struct tmat3x3
{
    using scalar_t = T;
    using vector_t = tvec3<T>;
    static const int vector_length = 9;

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

    static constexpr tmat3x3 zero()
    {
        return{ {
            { T(0), T(0), T(0) },
            { T(0), T(0), T(0) },
            { T(0), T(0), T(0) },
        } };
    }
    static constexpr tmat3x3 identity()
    {
        return{ {
            { T(1), T(0), T(0) },
            { T(0), T(1), T(0) },
            { T(0), T(0), T(1) },
        } };
    }
};

template<class T>
struct tmat4x4
{
    using scalar_t = T;
    using vector_t = tvec4<T>;
    static const int vector_length = 16;

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

    static constexpr tmat4x4 zero()
    {
        return{ {
            { (T)0, (T)0, (T)0, (T)0 },
            { (T)0, (T)0, (T)0, (T)0 },
            { (T)0, (T)0, (T)0, (T)0 },
            { (T)0, (T)0, (T)0, (T)0 },
        } };
    }
    static constexpr tmat4x4 identity()
    {
        return{ {
            { (T)1, (T)0, (T)0, (T)0 },
            { (T)0, (T)1, (T)0, (T)0 },
            { (T)0, (T)0, (T)1, (T)0 },
            { (T)0, (T)0, (T)0, (T)1 },
        } };
    }
};

template<class T> struct get_scalar_type_t { using type = typename T::scalar_t; };
#define Scalar(T) template<> struct get_scalar_type_t<T> { using type = T; }
Scalar(int); Scalar(half); Scalar(float); Scalar(double);
Scalar(snorm8); Scalar(unorm8); Scalar(unorm8n); Scalar(snorm16); Scalar(unorm16);
#undef Scalar
template<class T> using get_scalar_type = typename get_scalar_type_t<T>::type;

template<class T> struct get_vector_type_t { using type = typename T::vector_t; };
template<class T> struct get_vector_type_t<tvec2<T>> { using type = tvec2<T>; };
template<class T> struct get_vector_type_t<tvec3<T>> { using type = tvec3<T>; };
template<class T> struct get_vector_type_t<tvec4<T>> { using type = tvec4<T>; };
template<class T> using get_vector_type = typename get_vector_type_t<T>::type;

using float2 = tvec2<float>;
using float3 = tvec3<float>;
using float4 = tvec4<float>;
using quatf = tquat<float>;
using float2x2 = tmat2x2<float>;
using float3x3 = tmat3x3<float>;
using float4x4 = tmat4x4<float>;

using double2 = tvec2<double>;
using double3 = tvec3<double>;
using double4 = tvec4<double>;
using quatd = tquat<double>;
using double2x2 = tmat2x2<double>;
using double3x3 = tmat3x3<double>;
using double4x4 = tmat4x4<double>;

using half2 = tvec2<half>;
using half3 = tvec3<half>;
using half4 = tvec4<half>;
using quath = tquat<half>;
using half2x2 = tmat2x2<half>;
using half3x3 = tmat3x3<half>;
using half4x4 = tmat4x4<half>;

using snorm8x2 = tvec2<snorm8>;
using snorm8x3 = tvec3<snorm8>;
using snorm8x4 = tvec4<snorm8>;

using unorm8x2 = tvec2<unorm8>;
using unorm8x3 = tvec3<unorm8>;
using unorm8x4 = tvec4<unorm8>;

using unorm8nx2 = tvec2<unorm8n>;
using unorm8nx3 = tvec3<unorm8n>;
using unorm8nx4 = tvec4<unorm8n>;

using snorm16x2 = tvec2<snorm16>;
using snorm16x3 = tvec3<snorm16>;
using snorm16x4 = tvec4<snorm16>;

using unorm16x2 = tvec2<unorm16>;
using unorm16x3 = tvec3<unorm16>;
using unorm16x4 = tvec4<unorm16>;

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

template<class T> inline tmat3x3<T> operator-(const tmat3x3<T>& v) { return{ -v[0], -v[1], -v[2] }; }
template<class T> inline tmat4x4<T> operator-(const tmat4x4<T>& v) { return{ -v[0], -v[1], -v[2], -v[3] }; }

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

template<class T> inline T sum(const tvec3<T>& v);
template<class T> inline T sum(const tvec4<T>& v);
template<class T> inline tmat3x3<T> transpose(const tmat3x3<T>& v);
template<class T> inline tmat4x4<T> transpose(const tmat4x4<T>& v);

template<class T> inline tmat3x3<T> operator*(const tmat3x3<T> &a, const tmat3x3<T> &b_)
{
    const auto b = transpose(b_);
    return { {
        { sum(a[0] * b[0]), sum(a[0] * b[1]), sum(a[0] * b[2]) },
        { sum(a[1] * b[0]), sum(a[1] * b[1]), sum(a[1] * b[2]) },
        { sum(a[2] * b[0]), sum(a[2] * b[1]), sum(a[2] * b[2]) },
    } };
}
template<class T> inline tmat3x3<T>& operator*=(tmat3x3<T>& a, const tmat3x3<T> &b)
{
    a = a * b;
    return a;
}

template<class T> inline tmat4x4<T> operator*(const tmat4x4<T> &a, const tmat4x4<T> &b_)
{
    const auto b = transpose(b_);
    return { {
        { sum(a[0] * b[0]), sum(a[0] * b[1]), sum(a[0] * b[2]), sum(a[0] * b[3]) },
        { sum(a[1] * b[0]), sum(a[1] * b[1]), sum(a[1] * b[2]), sum(a[1] * b[3]) },
        { sum(a[2] * b[0]), sum(a[2] * b[1]), sum(a[2] * b[2]), sum(a[2] * b[3]) },
        { sum(a[3] * b[0]), sum(a[3] * b[1]), sum(a[3] * b[2]), sum(a[3] * b[3]) },
    }};
}
template<class T> inline tmat4x4<T>& operator*=(tmat4x4<T>& a, const tmat4x4<T> &b)
{
    a = a * b;
    return a;
}

inline int ceildiv(int v, int d) { return (v + (d - 1)) / d; }
inline int clamp(int v, int vmin, int vmax) { return std::min(std::max(v, vmin), vmax); }

#define SF(T)                                                                                       \
    inline T sign(T v) { return v < T(0.0) ? T(-1.0) : T(1.0); }                                    \
    inline T rcp(T v) { return T(1.0) / v; }                                                        \
    inline T rsqrt(T v) { return T(1.0) / std::sqrt(v); }                                           \
    inline T mod(T a, T b) { return std::fmod(a, b); }                                              \
    inline T frac(T a) { return std::fmod(a, T(1.0)); }                                             \
    inline T clamp(T v, T vmin, T vmax) { return std::min<T>(std::max<T>(v, vmin), vmax); }         \
    inline T clamp01(T v) { return clamp(v, T(0), T(1)); }                                          \
    inline T clamp11(T v) { return clamp(v, T(-1), T(1)); }                                         \
    inline T lerp(T  a, T  b, T  t) { return a * (T(1.0) - t) + b * t; }                            \
    inline bool near_equal(T a, T b, T epsilon = T(muEpsilon)) { return std::abs(a - b) < epsilon; }\

SF(float)
SF(double)
#undef SF

#define VF1N(N, F)\
    template<class T> inline tvec2<T> N(const tvec2<T>& a) { return{ F(a.x), F(a.y) }; }\
    template<class T> inline tvec3<T> N(const tvec3<T>& a) { return{ F(a.x), F(a.y), F(a.z) }; }\
    template<class T> inline tvec4<T> N(const tvec4<T>& a) { return{ F(a.x), F(a.y), F(a.z), F(a.w) }; }\

#define VF2N(N, F)\
    template<class T> inline tvec2<T> N(const tvec2<T>& a, const tvec2<T>& b) { return{ F(a.x, b.x), F(a.y, b.y) }; }\
    template<class T> inline tvec3<T> N(const tvec3<T>& a, const tvec3<T>& b) { return{ F(a.x, b.x), F(a.y, b.y), F(a.z, b.z) }; }\
    template<class T> inline tvec4<T> N(const tvec4<T>& a, const tvec4<T>& b) { return{ F(a.x, b.x), F(a.y, b.y), F(a.z, b.z), F(a.w, b.w) }; }\
    template<class T> inline tvec2<T> N(const tvec2<T>& a, T b) { return{ F(a.x, b), F(a.y, b) }; }\
    template<class T> inline tvec3<T> N(const tvec3<T>& a, T b) { return{ F(a.x, b), F(a.y, b), F(a.z, b) }; }\
    template<class T> inline tvec4<T> N(const tvec4<T>& a, T b) { return{ F(a.x, b), F(a.y, b), F(a.z, b), F(a.w, b) }; }

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
VF1(sign)
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
VF1(clamp11)

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
template<class T> inline bool near_equal(const tmat2x2<T>& a, const tmat2x2<T>& b, T e = muEpsilon)
{
    return near_equal(a[0], b[0], e) && near_equal(a[1], b[1], e);
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
template<class T> inline T dot(const tvec4<T>& l, const tvec4<T>& r) { return l.x*r.x + l.y*r.y + l.z*r.z + l.w*r.w; }
template<class T> inline T dot(const tquat<T>& l, const tquat<T>& r) { return dot((const float4&)l, (const float4&)r); }
template<class T> inline T length_sq(const tvec2<T>& v) { return dot(v, v); }
template<class T> inline T length_sq(const tvec3<T>& v) { return dot(v, v); }
template<class T> inline T length_sq(const tvec4<T>& v) { return dot(v, v); }
template<class T> inline T length(const tvec2<T>& v) { return sqrt(length_sq(v)); }
template<class T> inline T length(const tvec3<T>& v) { return sqrt(length_sq(v)); }
template<class T> inline T length(const tvec4<T>& v) { return sqrt(length_sq(v)); }
template<class T> inline tvec2<T> normalize(const tvec2<T>& v) { return v / length(v); }
template<class T> inline tvec3<T> normalize(const tvec3<T>& v) { return v / length(v); }
template<class T> inline tvec4<T> normalize(const tvec4<T>& v) { return v / length(v); }
template<class T> inline tquat<T> normalize(const tquat<T>& v)
{
    auto r = normalize((const tvec4<T>&)v);
    return (const tquat<T>&)r;
}
template<class T> inline tvec3<T> cross(const tvec3<T>& l, const tvec3<T>& r)
{
    return{
        l.y * r.z - l.z * r.y,
        l.z * r.x - l.x * r.z,
        l.x * r.y - l.y * r.x };
}

template<class T> inline T sum(const tvec2<T>& v) { return v.x + v.y; }
template<class T> inline T sum(const tvec3<T>& v) { return v.x + v.y + v.z; }
template<class T> inline T sum(const tvec4<T>& v) { return v.x + v.y + v.z + v.w; }


// a & b must be normalized
template<class T> inline T angle_between(const tvec3<T>& a, const tvec3<T>& b)
{
    return acos(clamp01(dot(a, b)));
}
template<class T> inline T angle_between2(const tvec3<T>& p1, const tvec3<T>& p2, const tvec3<T>& center)
{
    return angle_between(
        normalize(p1 - center),
        normalize(p2 - center));
}

template<class T> inline T angle_between_signed(const tvec3<T>& a, const tvec3<T>& b, const tvec3<T>& n)
{
    float ret = atan2(length(cross(a, b)), dot(a, b));
    if (dot(n, cross(a, b)) < 0)
        ret *= -1.0f;
    return ret;
}
template<class T> inline T angle_between2_signed(const tvec3<T>& p1, const tvec3<T>& p2, const tvec3<T>& center, const tvec3<T>& n)
{
    return angle_between_signed((p1 - center), (p2 - center), n);
}

template<class T> inline tquat<T> lerp(const tquat<T>& q1, const tquat<T>& q2, float t)
{
    tquat<T> ret;
    if (dot(q1, q2) < T(0.0)) {
        ret = {
            q1.x + t * (-q2.x - q1.x),
            q1.y + t * (-q2.y - q1.y),
            q1.z + t * (-q2.z - q1.z),
            q1.w + t * (-q2.w - q1.w)
        };
    }
    else {
        ret = {
            q1.x + t * (q2.x - q1.x),
            q1.y + t * (q2.y - q1.y),
            q1.z + t * (q2.z - q1.z),
            q1.w + t * (q2.w - q1.w)
        };
    }
    return normalize(ret);
}

template<class T> inline tquat<T> slerp(const tquat<T>& q1, const tquat<T>& q2, T t)
{
    float d = dot(q1, q2);

    tquat<T> tmp;
    if (d < T(0.0)) {
        d = -d;
        tmp = { -q2.x, -q2.y, -q2.z, -q2.w };
    }
    else
        tmp = q2;

    if (d < T(0.95)) {
        T angle = std::acos(d);
        T sinadiv = T(1.0) / std::sin(angle);
        T sinat = std::sin(angle * t);
        T sinaomt = std::sin(angle * (T(1.0) - t));
        return {
            (q1.x * sinaomt + tmp.x * sinat) * sinadiv,
            (q1.y * sinaomt + tmp.y * sinat) * sinadiv,
            (q1.z * sinaomt + tmp.z * sinat) * sinadiv,
            (q1.w * sinaomt + tmp.w * sinat) * sinadiv
        };
    }
    else {
        return lerp(q1, tmp, t);
    }
}

template<class T> inline tvec3<T> apply_rotation(const tquat<T>& q, const tvec3<T>& p)
{
    auto a = cross(reinterpret_cast<const tvec3<T>&>(q), p);
    auto b = cross(reinterpret_cast<const tvec3<T>&>(q), a);
    return p + (a * q.w + b) * T(2.0);
}

template<class T> inline tquat<T> invert(const tquat<T>& v)
{
    return{ -v.x, -v.y, -v.z, v.w };
}

template<class T> inline tquat<T> rotate_x(T angle)
{
    T c = cos(angle * T(0.5));
    T s = sin(angle * T(0.5));
    return{ s, T(0.0), T(0.0), c };
}
template<class T> inline tquat<T> rotate_y(T angle)
{
    T c = cos(angle * T(0.5));
    T s = sin(angle * T(0.5));
    return{ T(0.0), s, T(0.0), c };
}
template<class T> inline tquat<T> rotate_z(T angle)
{
    T c = cos(angle * T(0.5));
    T s = sin(angle * T(0.5));
    return{ T(0.0), T(0.0), s, c };
}

// euler -> quaternion
template<class T> inline tquat<T> rotate_xyz(const tvec3<T>& euler)
{
    auto qX = rotate_x(euler.x);
    auto qY = rotate_y(euler.y);
    auto qZ = rotate_z(euler.z);
    return (qZ * qY) * qX;
}
template<class T> inline tquat<T> rotate_xzy(const tvec3<T>& euler)
{
    auto qX = rotate_x(euler.x);
    auto qY = rotate_y(euler.y);
    auto qZ = rotate_z(euler.z);
    return (qY * qZ) * qX;
}
template<class T> inline tquat<T> rotate_yxz(const tvec3<T>& euler)
{
    auto qX = rotate_x(euler.x);
    auto qY = rotate_y(euler.y);
    auto qZ = rotate_z(euler.z);
    return (qZ * qX) * qY;
}
template<class T> inline tquat<T> rotate_yzx(const tvec3<T>& euler)
{
    auto qX = rotate_x(euler.x);
    auto qY = rotate_y(euler.y);
    auto qZ = rotate_z(euler.z);
    return (qX * qZ) * qY;
}
template<class T> inline tquat<T> rotate_zxy(const tvec3<T>& euler)
{
    auto qX = rotate_x(euler.x);
    auto qY = rotate_y(euler.y);
    auto qZ = rotate_z(euler.z);
    return (qY * qX) * qZ;
}
template<class T> inline tquat<T> rotate_zyx(const tvec3<T>& euler)
{
    auto qX = rotate_x(euler.x);
    auto qY = rotate_y(euler.y);
    auto qZ = rotate_z(euler.z);
    return (qX * qY) * qZ;
}

template<class T> inline tquat<T> rotate(const tvec3<T>& axis, T angle)
{
    return{
        axis.x * sin(angle * T(0.5)),
        axis.y * sin(angle * T(0.5)),
        axis.z * sin(angle * T(0.5)),
        cos(angle * T(0.5))
    };
}

template<class T> inline tvec3<T> to_euler_zxy(const tquat<T>& q)
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

    if (abs(v0) < T(0.499999))
    {
        T v5 = T(2.0) * (d[2] + d[6]);
        T v6 = d[7] - d[0] - d[4] + d[9];

        return{
            v3 * asin(clamp11(v4)),
            atan2(v5, v6),
            atan2(v1, v2)
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
            v3 * asin(clamp11(v4)),
            atan2(v5, v6),
            T(0.0)
        };
    }
}

template<class T> inline void to_axis_angle(const tquat<T>& q, tvec3<T>& axis, T& angle)
{
    angle = T(2.0) * acos(q.w);
    axis = {
        q.x / sqrt(T(1.0) - q.w*q.w),
        q.y / sqrt(T(1.0) - q.w*q.w),
        q.z / sqrt(T(1.0) - q.w*q.w)
    };
}

template<class T> inline tmat3x3<T> look33(const tvec3<T>& forward, const tvec3<T>& up)
{
    auto z = normalize(forward);
    auto x = normalize(cross(up, z));
    auto y = cross(z, x);
    return{ {
        { x.x, y.x, z.x },
        { x.y, y.y, z.y },
        { x.z, y.z, z.z },
    } };
}
template<class T> inline tmat4x4<T> look44(const tvec3<T>& forward, const tvec3<T>& up)
{
    auto z = normalize(forward);
    auto x = normalize(cross(up, z));
    auto y = cross(z, x);
    return{ {
        { x.x, y.x, z.x, T(0) },
        { x.y, y.y, z.y, T(0) },
        { x.z, y.z, z.z, T(0) },
        {T(0),T(0),T(0), T(1) },
    } };
}

template<class T> inline tvec3<T> flip_x(const tvec3<T>& v) { return { -v.x, v.y, v.z }; }
template<class T> inline tvec4<T> flip_x(const tvec4<T>& v) { return { -v.x, v.y, v.z, v.w }; }
template<class T> inline tquat<T> flip_x(const tquat<T>& v) { return { v.x, -v.y, -v.z, v.w }; }
template<class T> inline tmat3x3<T> flip_x(const tmat3x3<T>& m)
{
    return tmat3x3<T> {
         m[0].x,-m[0].y,-m[0].z,
        -m[1].x, m[1].y, m[1].z,
        -m[2].x, m[2].y, m[2].z,
    };
}
template<class T> inline tmat4x4<T> flip_x(const tmat4x4<T>& m)
{
    return tmat4x4<T> {
         m[0].x,-m[0].y,-m[0].z,-m[0].w,
        -m[1].x, m[1].y, m[1].z, m[1].w,
        -m[2].x, m[2].y, m[2].z, m[2].w,
        -m[3].x, m[3].y, m[3].z, m[3].w,
    };
}

template<class T> inline tvec3<T> flip_y(const tvec3<T>& v) { return { v.x, -v.y, v.z }; }
template<class T> inline tvec4<T> flip_y(const tvec4<T>& v) { return { v.x, -v.y, v.z, v.w }; }
template<class T> inline tquat<T> flip_y(const tquat<T>& v) { return { -v.x, v.y, -v.z, v.w }; }
template<class T> inline tmat3x3<T> flip_y(const tmat3x3<T>& m)
{
    return tmat3x3<T> {
         m[0].x,-m[0].y, m[0].z,
        -m[1].x, m[1].y,-m[1].z,
         m[2].x,-m[2].y, m[2].z,
    };
}
template<class T> inline tmat4x4<T> flip_y(const tmat4x4<T>& m)
{
    return tmat4x4<T> {
         m[0].x,-m[0].y, m[0].z, m[0].w,
        -m[1].x, m[1].y,-m[1].z,-m[1].w,
         m[2].x,-m[2].y, m[2].z, m[2].w,
         m[3].x,-m[3].y, m[3].z, m[3].w,
    };
}

template<class T> inline tvec2<T> flip_z(const tvec2<T>& v) { return { v.x, v.y }; }
template<class T> inline tvec3<T> flip_z(const tvec3<T>& v) { return { v.x, v.y, -v.z }; }
template<class T> inline tvec4<T> flip_z(const tvec4<T>& v) { return { v.x, v.y, -v.z, v.w }; }
template<class T> inline tquat<T> flip_z(const tquat<T>& v) { return { -v.x, -v.y, v.z, v.w }; }
template<class T> inline tmat3x3<T> flip_z(const tmat3x3<T>& m)
{
    return tmat3x3<T> {
        m[0].x, m[0].y,-m[0].z,
        m[1].x, m[1].y,-m[1].z,
       -m[2].x,-m[2].y, m[2].z,
    };
}
template<class T> inline tmat4x4<T> flip_z(const tmat4x4<T>& m)
{
    return tmat4x4<T> {
        m[0].x, m[0].y,-m[0].z, m[0].w,
        m[1].x, m[1].y,-m[1].z, m[1].w,
       -m[2].x,-m[2].y, m[2].z,-m[2].w,
        m[3].x, m[3].y,-m[3].z, m[3].w,
    };
}

template<class T> inline tvec3<T> swap_yz(const tvec3<T>& v) { return { v.x, v.z, v.y }; }
template<class T> inline tvec4<T> swap_yz(const tvec4<T>& v) { return { v.x, v.z, v.y, v.w }; }
template<class T> inline tquat<T> swap_yz(const tquat<T>& v) { return {-v.x,-v.z,-v.y, v.w }; }
template<class T> inline tmat3x3<T> swap_yz(const tmat3x3<T>& m)
{
    return tmat3x3<T> {
        m[0].x, m[0].z, m[0].y,
        m[2].x, m[2].z, m[2].y,
        m[1].x, m[1].z, m[1].y,
    };
}
template<class T> inline tmat4x4<T> swap_yz(const tmat4x4<T>& m)
{
    return tmat4x4<T> {
        m[0].x, m[0].z, m[0].y, m[0].w,
        m[2].x, m[2].z, m[2].y, m[2].w,
        m[1].x, m[1].z, m[1].y, m[1].w,
        m[3].x, m[3].z, m[3].y, m[3].w,
    };
}

// convert to different type of vec/mat with same length (e.g. float3 <-> half3, float4x4 <-> double4x4)
template<class T, class U> inline T to(const tvec2<U>& v)   { T r; r.assign(v); return r; }
template<class T, class U> inline T to(const tvec3<U>& v)   { T r; r.assign(v); return r; }
template<class T, class U> inline T to(const tvec4<U>& v)   { T r; r.assign(v); return r; }
template<class T, class U> inline T to(const tquat<U>& v)   { T r; r.assign(v); return r; }
template<class T, class U> inline T to(const tmat2x2<U>& v) { T r; r.assign(v); return r; }
template<class T, class U> inline T to(const tmat3x3<U>& v) { T r; r.assign(v); return r; }
template<class T, class U> inline T to(const tmat4x4<U>& v) { T r; r.assign(v); return r; }

// convert to same type of vec/mat with different length
template<class T> inline tvec3<T> to_vec3(const tvec2<T>& v) { return { v[0], v[1], 0 }; }
template<class T> inline tvec3<T> to_vec3(const tvec3<T>& v) { return v; }
template<class T> inline tvec3<T> to_vec3(const tvec4<T>& v) { return { v[0], v[1], v[2] }; }
template<class T> inline tvec4<T> to_vec4(const tvec2<T>& v) { return { v[0], v[1], 0, 0}; }
template<class T> inline tvec4<T> to_vec4(const tvec3<T>& v) { return { v[0], v[1], v[2], 0 }; }
template<class T> inline tvec4<T> to_vec4(const tvec4<T>& v) { return v; }

template<class T> inline tmat2x2<T> to_mat2x2(const tmat2x2<T>& v)
{
    return v;
}
template<class T> inline tmat2x2<T> to_mat2x2(const tmat3x3<T>& v)
{
    return tmat2x2<T>{(tvec2<T>&)v[0], (tvec2<T>&)v[1]};
}
template<class T> inline tmat2x2<T> to_mat2x2(const tmat4x4<T>& v)
{
    return tmat2x2<T>{(tvec2<T>&)v[0], (tvec2<T>&)v[1]};
}

template<class T> inline tmat3x3<T> to_mat3x3(const tmat2x2<T>& v)
{
    return tmat3x3<T>{
        v[0][0], v[0][1], 0,
        v[1][0], v[1][1], 0,
              0,       0, 1,
    };
}
template<class T> inline tmat3x3<T> to_mat3x3(const tmat3x3<T>& v)
{
    return v;
}
template<class T> inline tmat3x3<T> to_mat3x3(const tmat4x4<T>& v)
{
    return tmat3x3<T>{(tvec3<T>&)v[0], (tvec3<T>&)v[1], (tvec3<T>&)v[2]};
}

template<class T> inline tmat4x4<T> to_mat4x4(const tmat2x2<T>& v)
{
    return tmat4x4<T>{
        v[0][0], v[0][1], 0, 0,
        v[1][0], v[1][1], 0, 0,
              0,       0, 1, 0,
              0,       0, 0, 1
    };
}
template<class T> inline tmat4x4<T> to_mat4x4(const tmat3x3<T>& v)
{
    return tmat4x4<T>{
        v[0][0], v[0][1], v[0][2], 0,
        v[1][0], v[1][1], v[1][2], 0,
        v[2][0], v[2][1], v[2][2], 0,
              0,       0,       0, 1
    };
}
template<class T> inline tmat4x4<T> to_mat4x4(const tmat4x4<T>& v)
{
    return v;
}

template<class T> inline tmat3x3<T> to_mat3x3(const tquat<T>& q)
{
    return tmat3x3<T>{{
        {T(1.0)-T(2.0)*q.y*q.y - T(2.0)*q.z*q.z,T(2.0)*q.x*q.y - T(2.0)*q.z*q.w,         T(2.0)*q.x*q.z + T(2.0)*q.y*q.w,        },
        {T(2.0)*q.x*q.y + T(2.0)*q.z*q.w,       T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.z*q.z,T(2.0)*q.y*q.z - T(2.0)*q.x*q.w,        },
        {T(2.0)*q.x*q.z - T(2.0)*q.y*q.w,       T(2.0)*q.y*q.z + T(2.0)*q.x*q.w,         T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.y*q.y}
    }};
}
template<class T> inline tmat4x4<T> to_mat4x4(const tquat<T>& q)
{
    return tmat4x4<T>{{
        {T(1.0)-T(2.0)*q.y*q.y - T(2.0)*q.z*q.z,T(2.0)*q.x*q.y - T(2.0)*q.z*q.w,         T(2.0)*q.x*q.z + T(2.0)*q.y*q.w,         T(0.0)},
        {T(2.0)*q.x*q.y + T(2.0)*q.z*q.w,       T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.z*q.z,T(2.0)*q.y*q.z - T(2.0)*q.x*q.w,         T(0.0)},
        {T(2.0)*q.x*q.z - T(2.0)*q.y*q.w,       T(2.0)*q.y*q.z + T(2.0)*q.x*q.w,         T(1.0) - T(2.0)*q.x*q.x - T(2.0)*q.y*q.y,T(0.0)},
        {T(0.0),                                T(0.0),                                  T(0.0),                                  T(1.0)}
    }};
}

template<class T> inline bool is_negative(const tmat3x3<T>& m)
{
    return dot(cross(m[0], m[1]), m[2]) < 0;
}
template<class T> inline bool is_negative(const tmat4x4<T>& m)
{
    return dot(cross((tvec3<T>&)m[0], (tvec3<T>&)m[1]), (tvec3<T>&)m[2]) < 0;
}

template<class T> inline tmat4x4<T> translate(const tvec3<T>& t)
{
    return tmat4x4<T>{{
        {T(1.0), T(0.0), T(0.0), T(0.0)},
        {T(0.0), T(1.0), T(0.0), T(0.0)},
        {T(0.0), T(0.0), T(1.0), T(0.0)},
        {   t.x,    t.y,    t.z, T(1.0)}
    }};
}

template<class T> inline tmat3x3<T> scale33(const tvec3<T>& t)
{
    return tmat3x3<T>{
           t.x, T(0.0), T(0.0),
        T(0.0),    t.y, T(0.0),
        T(0.0), T(0.0),    t.z,
    };
}
template<class T> inline tmat4x4<T> scale44(const tvec3<T>& t)
{
    return tmat4x4<T>{
           t.x, T(0.0), T(0.0), T(0.0),
        T(0.0),    t.y, T(0.0), T(0.0),
        T(0.0), T(0.0),    t.z, T(0.0),
        T(0.0), T(0.0), T(0.0), T(1.0)
    };
}

template<class T> inline tmat4x4<T> transform(const tvec3<T>& t, const tquat<T>& r, const tvec3<T>& s)
{
    auto ret = scale44(s);
    ret *= to_mat4x4(r);
    (tvec3<T>&)ret[3] = t;
    return ret;
}

template<class T> inline tmat3x3<T> transpose(const tmat3x3<T>& x)
{
    return tmat3x3<T>{
        x[0][0], x[1][0], x[2][0],
        x[0][1], x[1][1], x[2][1],
        x[0][2], x[1][2], x[2][2],
    };
}
template<class T> inline tmat4x4<T> transpose(const tmat4x4<T>& x)
{
    return tmat4x4<T>{
        x[0][0], x[1][0], x[2][0], x[3][0],
        x[0][1], x[1][1], x[2][1], x[3][1],
        x[0][2], x[1][2], x[2][2], x[3][2],
        x[0][3], x[1][3], x[2][3], x[3][3],
    };
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

        if (abs(r) >= T(1.0)) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    s[i][j] /= r;
                }
            }
        }
        else {
            T mr = abs(r) / std::numeric_limits<T>::min();

            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    if (mr > abs(s[i][j])) {
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

        if (abs(r) >= 1) {
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    s[i][j] /= r;
                }
            }
        }
        else {
            T mr = abs(r) / std::numeric_limits<T>::min();

            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    if (mr > abs(s[i][j])) {
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

    if (abs(r) >= 1) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                s[i][j] /= r;
            }
        }
    }
    else {
        auto mr = abs(r) / std::numeric_limits<T>::min();

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (mr > abs(s[i][j])) {
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

template <typename T>
inline tmat4x4<T> look_at(const tvec3<T>& eye, const tvec3<T>& target, const tvec3<T>& up)
{
    tvec3<T> f = { normalize(target - eye) };
    tvec3<T> r = { normalize(cross(f, up)) };
    tvec3<T> u = { cross(r, f) };
    tvec3<T> p = { -dot(r, eye), -dot(u, eye), dot(f, eye) };

    return { {
        { r.x, u.x, -f.x, T(0.0) },
        { r.y, u.y, -f.y, T(0.0) },
        { r.z, u.z, -f.z, T(0.0) },
        { p.x, p.y,  p.z, T(1.0) },
    } };
}

template<class T>
inline void extract_projection_data(const tmat4x4<T>& proj, T& fov, T& aspect, T& near_plane, T& far_plane)
{
    auto tan_half_fov = T(1.0) / proj[1][1];
    fov = atan(tan_half_fov) * T(2.0) * RadToDeg;
    aspect = (T(1.0) / proj[0][0]) / tan_half_fov;

    auto m22 = -proj[2][2];
    auto m32 = -proj[3][2];
    auto tmp_near = abs((T(2.0) * m32) / (T(2.0)*m22 - T(2.0)));
    auto tmp_far = abs(((m22 - T(1.0))*tmp_near) / (m22 + T(1.0)));
    if (tmp_near > tmp_far)
        std::swap(tmp_near, tmp_far);
    near_plane = tmp_near;
    far_plane = tmp_far;
}

template<class T>
inline void extract_look_data(const tmat4x4<T>& view, tvec3<T>& pos, tvec3<T>& forward, tvec3<T>& up, tvec3<T>& right)
{
    auto tview = transpose(view);

    auto n1 = (tvec3<T>&)tview[0];
    auto n2 = (tvec3<T>&)tview[1];
    auto n3 = (tvec3<T>&)tview[2];
    auto d1 = tview[0].w;
    auto d2 = tview[1].w;
    auto d3 = tview[2].w;
    auto n2n3 = cross(n2, n3);
    auto n3n1 = cross(n3, n1);
    auto n1n2 = cross(n1, n2);
    auto top = (n2n3 * d1) + (n3n1 * d2) + (n1n2 * d3);
    auto denom = dot(n1, n2n3);
    pos = top / -denom;

    forward = (tvec3<T>&)tview[2];
    up = (tvec3<T>&)tview[1];
    right = (tvec3<T>&)tview[0];
}
template<class T>
inline void extract_look_data(const tmat4x4<T>& view, tvec3<T>& pos, tquat<T>& rot)
{
    tvec3<T> forward, up, right;
    extract_look_data(view, pos, forward, up, right);
    rot = to_quat(look33(forward, up));
}


template<class TMat>
inline tquat<typename TMat::scalar_t> to_quat_impl(const TMat& m_)
{
    using T = typename TMat::scalar_t;
    tmat3x3<T> m {
        normalize((tvec3<T>&)m_[0]),
        normalize((tvec3<T>&)m_[1]),
        normalize((tvec3<T>&)m_[2])
    };

    T trace = m[0][0] + m[1][1] + m[2][2];
    T root;
    tquat<T> q;

    if (trace > 0.0f)
    {
        // |w| > 1/2, may as well choose w > 1/2
        root = sqrt(trace + 1.0f);   // 2w
        q.w = 0.5f * root;
        root = 0.5f / root;  // 1/(4w)
        q.x = (m[2][1] - m[1][2]) * root;
        q.y = (m[0][2] - m[2][0]) * root;
        q.z = (m[1][0] - m[0][1]) * root;
    }
    else
    {
        // |w| <= 1/2
        int next[3] = { 1, 2, 0 };
        int i = 0;
        if (m[1][1] > m[0][0])
            i = 1;
        if (m[2][2] > m[i][i])
            i = 2;
        int j = next[i];
        int k = next[j];

        root = sqrt(m[i][i] - m[j][j] - m[k][k] + T(1.0));
        float* qv[3] = { &q.x, &q.y, &q.z };
        *qv[i] = T(0.5) * root;
        root = T(0.5) / root;
        q.w = (m[k][j] - m[j][k]) * root;
        *qv[j] = (m[j][i] + m[i][j]) * root;
        *qv[k] = (m[k][i] + m[i][k]) * root;
    }
    q = normalize(q);
    return q;
}

template<class T> inline tquat<T> to_quat(const tmat3x3<T>& m)
{
    return to_quat_impl(m);
}
template<class T> inline tquat<T> to_quat(const tmat4x4<T>& m)
{
    return to_quat_impl(m);
}

template<class T> inline tvec3<T> extract_position(const tmat4x4<T>& m)
{
    return (const tvec3<T>&)m[3];
}


template<class T> inline tvec3<T> extract_scale_unsigned(const tmat3x3<T>& m)
{
    return tvec3<T>{length(m[0]), length(m[1]), length(m[2])};
}
template<class T> inline tvec3<T> extract_scale_unsigned(const tmat4x4<T>& m)
{
    return tvec3<T>{length((tvec3<T>&)m[0]), length((tvec3<T>&)m[1]), length((tvec3<T>&)m[2])};
}

template<class TMat>
inline typename TMat::scalar_t extract_scale_sign(const TMat& m)
{
    using T = typename TMat::scalar_t;
    auto ax = (const tvec3<T>&)m[0];
    auto ay = (const tvec3<T>&)m[1];
    auto az = (const tvec3<T>&)m[2];
    return sign(dot(cross(ax, ay), az));
}

template<class TMat>
inline tvec3<typename TMat::scalar_t> extract_scale_signed_impl(const TMat& m)
{
    using T = typename TMat::scalar_t;
    auto s = extract_scale_sign(m);
    return tvec3<T>{ length(m[0]) * s, length(m[1]) * s, length(m[2]) * s };
}

template<class T> inline tvec3<T> extract_scale(const tmat3x3<T>& m)
{
    return extract_scale_signed_impl(m);
}
template<class T> inline tvec3<T> extract_scale(const tmat4x4<T>& m)
{
    return extract_scale_signed_impl(m);
}

template<class TMat>
inline tquat<typename TMat::scalar_t> extract_rotation_impl(const TMat& m_)
{
    using T = typename TMat::scalar_t;
    tmat3x3<T> m{
        normalize((tvec3<T>&)m_[0]),
        normalize((tvec3<T>&)m_[1]),
        normalize((tvec3<T>&)m_[2])
    };
    {
        auto s = extract_scale_sign(m_);
        m[0] *= s;
        m[1] *= s;
        m[2] *= s;
    }

    tquat<T> q;
    T tr, s;

    tr = T(0.25) * (T(1) + m[0][0] + m[1][1] + m[2][2]);

    if (tr > T(1e-4f)) {
        s = sqrt(tr);
        q[3] = (float)s;
        s = T(1) / (T(4) * s);
        q[0] = (m[1][2] - m[2][1]) * s;
        q[1] = (m[2][0] - m[0][2]) * s;
        q[2] = (m[0][1] - m[1][0]) * s;
    }
    else {
        if (m[0][0] > m[1][1] && m[0][0] > m[2][2]) {
            s = T(2) * sqrt(T(1) + m[0][0] - m[1][1] - m[2][2]);
            q[0] = T(0.25) * s;

            s = T(1) / s;
            q[3] = (m[1][2] - m[2][1]) * s;
            q[1] = (m[1][0] + m[0][1]) * s;
            q[2] = (m[2][0] + m[0][2]) * s;
        }
        else if (m[1][1] > m[2][2]) {
            s = T(2) * sqrt(T(1) + m[1][1] - m[0][0] - m[2][2]);
            q[1] = T(0.25) * s;

            s = T(1) / s;
            q[3] = (m[2][0] - m[0][2]) * s;
            q[0] = (m[1][0] + m[0][1]) * s;
            q[2] = (m[2][1] + m[1][2]) * s;
        }
        else {
            s = T(2) * sqrt(T(1) + m[2][2] - m[0][0] - m[1][1]);
            q[2] = T(0.25) * s;

            s = T(1) / s;
            q[3] = (m[0][1] - m[1][0]) * s;
            q[0] = (m[2][0] + m[0][2]) * s;
            q[1] = (m[2][1] + m[1][2]) * s;
        }
    }
    return normalize(q);
}
template<class T> inline tquat<T> extract_rotation(const tmat3x3<T>& m)
{
    return extract_rotation_impl(m);
}
template<class T> inline tquat<T> extract_rotation(const tmat4x4<T>& m)
{
    return extract_rotation_impl(m);
}
template<class T> inline void extract_trs(const tmat4x4<T>& m, tvec3<T>& t, tquat<T>& r, tvec3<T>& s)
{
    t = extract_position(m);
    r = extract_rotation(m);
    s = extract_scale(m);
    if (near_equal(s, tvec3<T>::one()))
        s = tvec3<T>::one();
}

template<class T> inline tmat4x4<T> cancel_s(const tmat4x4<T>& m)
{
    auto r = m;
    for (int i = 0; i < 3; ++i)
        r[i] = normalize(m[i]);
    return r;
}
template<class T> inline tmat4x4<T> cancel_rs(const tmat4x4<T>& m)
{
    auto r = tmat4x4<T>::identity();
    r[3] = m[3];
    return r;
}

// aperture and focal_length must be millimeter. return fov in degree
template<class T> inline T compute_fov(T aperture, T focal_length)
{
    return T(2.0) * atan(aperture / (T(2.0) * focal_length)) * RadToDeg;
}

// aperture: millimeter
// fov: degree
template<class T> inline T compute_focal_length(T aperture, T fov)
{
    return aperture / tan(fov * DegToRad / T(2.0)) / T(2.0);
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
    if (abs(det) < epsdet) return false;
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

    tvec3<T> tangent, binormal;
    T div = s.x * t.y - s.y * t.x;
    T area = abs(div);
    if (area > 1e-8f) {
        T rdiv = T(1.0) / div;
        s *= rdiv;
        t *= rdiv;

        tangent = normalize(tvec3<T>{
            t.y * p.x - t.x * q.x,
                t.y * p.y - t.x * q.y,
                t.y * p.z - t.x * q.z
        }) * area;
        binormal = normalize(tvec3<T>{
            s.x * q.x - s.y * p.x,
                s.x * q.y - s.y * p.y,
                s.x * q.z - s.y * p.z
        }) * area;
    }
    else {
        tangent = binormal = tvec3<T>::zero();
    }

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

    const auto epsilon = 1e-6f;
    if (magT <= epsilon || magB <= epsilon)
    {
        tvec3<T> axis1, axis2;

        auto dpXN = abs(dot({ T(1.0), T(0.0), T(0.0) }, normal));
        auto dpYN = abs(dot({ T(0.0), T(1.0), T(0.0) }, normal));
        auto dpZN = abs(dot({ T(0.0), T(0.0), T(1.0) }, normal));

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

    return { tangent.x, tangent.y, tangent.z,
        dot(cross(normal, tangent), binormal) > T(0.0) ? T(1.0) : -T(1.0) };
}

template<class A1, class Body>
static inline void enumerate(A1& a1, const Body& body)
{
    size_t n = a1.size();
    for (size_t i = 0; i < n; ++i)
        body(a1[i]);
}
template<class A1, class A2, class Body>
static inline void enumerate(A1& a1, A2& a2, const Body& body)
{
    size_t n = a1.size();
    if (n != a2.size())
        return;
    for (size_t i = 0; i < n; ++i)
        body(a1[i], a2[i]);
}
template<class A1, class A2, class A3, class Body>
static inline void enumerate(A1& a1, A2& a2, A3& a3, const Body& body)
{
    size_t n = a1.size();
    if (n != a2.size() || n != a3.size())
        return;
    for (size_t i = 0; i < n; ++i)
        body(a1[i], a2[i], a3[i]);
}

} // namespace mu
