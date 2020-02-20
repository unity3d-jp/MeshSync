#pragma once
#include "muMath.h"

namespace mu {

struct quat32
{
    static constexpr float SR2 = 1.41421356237f;
    static constexpr float RSR2 = 1.0f / 1.41421356237f;
    static constexpr float C = float(0x3ff);
    static constexpr float R = 1.0f / float(0x3ff);

    static constexpr uint32_t pack(float a)
    {
        return static_cast<uint32_t>((a * SR2 + 1.0f) * 0.5f * C);
    }
    static constexpr float unpack(uint32_t a)
    {
        return ((a * R) * 2.0f - 1.0f) * RSR2;
    }
    static constexpr float square(float a)
    {
        return a * a;
    }
    static int dropmax(float a, float b, float c, float d)
    {
        if (a > b && a > c && a > d) return 0;
        if (b > c && b > d) return 1;
        if (c > d) return 2;
        return 3;
    }

    struct
    {
        uint32_t x0 : 10;
        uint32_t x1 : 10;
        uint32_t x2 : 10;
        uint32_t drop : 2;
    } value;

    quat32() {}
    quat32(const quat32& v) : value(v.value) {}
    template<class T> quat32(const tquat<T>& v) { *this = v; }

    // assume v_ is normalized
    template<class T>
    quat32& operator=(const tquat<T>& v_)
    {
        quatf v = to<quatf>(v_);

        float a0, a1, a2;
        value.drop = dropmax(square(v[0]), square(v[1]), square(v[2]), square(v[3]));
        if (value.drop == 0) {
            float s = sign(v[0]);
            a0 = v[1] * s;
            a1 = v[2] * s;
            a2 = v[3] * s;
        }
        else if (value.drop == 1) {
            float s = sign(v[1]);
            a0 = v[0] * s;
            a1 = v[2] * s;
            a2 = v[3] * s;
        }
        else if (value.drop == 2) {
            float s = sign(v[2]);
            a0 = v[0] * s;
            a1 = v[1] * s;
            a2 = v[3] * s;
        }
        else {
            float s = sign(v[3]);
            a0 = v[0] * s;
            a1 = v[1] * s;
            a2 = v[2] * s;
        }

        value.x0 = pack(a0);
        value.x1 = pack(a1);
        value.x2 = pack(a2);
        return *this;
    }

    template<class T>
    operator tquat<T>() const
    {
        const float a0 = unpack(value.x0);
        const float a1 = unpack(value.x1);
        const float a2 = unpack(value.x2);
        const float iss = sqrt(1.0f - (square(a0) + square(a1) + square(a2)));

        switch (value.drop) {
        case 0: return { (T)iss, (T)a0, (T)a1, (T)a2 };
        case 1: return { (T)a0, (T)iss, (T)a1, (T)a2 };
        case 2: return { (T)a0, (T)a1, (T)iss, (T)a2 };
        default:return { (T)a0, (T)a1, (T)a2, (T)iss };
        }
    }

    static quat32 identity() { return quat32(quatf::identity()); }
};

template<class T> inline T to(quat32 v) { return (T)v; }


// 32bit normal/tangent
struct snormx3_32
{
    static constexpr float C = float(0x7fff);
    static constexpr float R = 1.0f / float(0x7fff);

    static uint32_t pack(float a)
    {
        return static_cast<uint32_t>((clamp11(a) * 0.5f + 0.5f) * C);
    }
    static float unpack(uint32_t a)
    {
        return a * R * 2.0f - 1.0f;
    }

    struct
    {
        uint32_t x : 15;
        uint32_t y : 15;
        uint32_t sz : 1;
        uint32_t sw : 1;
    } value;

    snormx3_32() {}
    snormx3_32(const snormx3_32& v) : value(v.value) {}
    template<class T> snormx3_32(const tvec3<T>& v) { *this = v; }
    template<class T> void assign(const tvec3<T>& v) { *this = v; }
    template<class T> snormx3_32(const tvec4<T>& v) { *this = v; }
    template<class T> void assign(const tvec4<T>& v) { *this = v; }

    template<class T>
    snormx3_32& operator=(const tvec3<T>& v_)
    {
        auto v = to<float3>(v_);
        (uint32_t&)value = 0;
        value.x = pack(v[0]);
        value.y = pack(v[1]);
        value.sz = v[2] < 0.0f ? 1 : 0;
        return *this;
    }

    template<class T>
    snormx3_32& operator=(const tvec4<T>& v_)
    {
        auto v = to<float4>(v_);
        (uint32_t&)value = 0;
        value.x = pack(v[0]);
        value.y = pack(v[1]);
        value.sz = v[2] < 0.0f ? 1 : 0;
        value.sw = v[3] < 0.0f ? 1 : 0;
        return *this;
    }

    template<class T>
    operator tvec3<T>() const
    {
        T _x = (T)unpack(value.x);
        T _y = (T)unpack(value.y);
        T _z = sqrt(1.0f - (_x*_x) - (_y*_y)) * (value.sz == 1 ? T(-1) : T(1));
        return { _x, _y, _z };
    }

    template<class T>
    operator tvec4<T>() const
    {
        T _x = (T)unpack(value.x);
        T _y = (T)unpack(value.y);
        T _z = sqrt(1.0f - (_x*_x) - (_y*_y)) * (value.sz == 1 ? T(-1) : T(1));
        T _w = (value.sw == 1 ? T(-1) : T(1));
        return { _x, _y, _z, _w };
    }

    static snormx3_32 zero() { return snormx3_32(float3::zero()); }
    static snormx3_32 one() { return snormx3_32(float3::one()); }
};

template<class T> inline T to(snormx3_32 v) { return (T)v; }

} // namespace mu
