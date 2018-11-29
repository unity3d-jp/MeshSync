#pragma once
#include "muMath.h"

namespace mu {

// -1.0f - 1.0f in 10 bits x 3
struct snorm10x3
{
    static constexpr float C = float(0x3ff);
    static constexpr float R = 1.0f / float(0x3ff);

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
        uint32_t x : 10;
        uint32_t y : 10;
        uint32_t z : 10;
        uint32_t w : 2;
    } value;

    snorm10x3() {}
    snorm10x3(const snorm10x3& v) : value(v.value) {}
    template<class T> snorm10x3(const tvec3<T>& v) { *this = v; }
    template<class T> void assign(const tvec3<T>& v) { *this = v; }

    template<class T>
    snorm10x3& operator=(const tvec3<T>& v_)
    {
        float3 v = to<float3>(v_);
        (uint32_t&)value = 0;
        value.x = pack(v[0]);
        value.y = pack(v[1]);
        value.z = pack(v[2]);
        return *this;
    }

    template<class T>
    operator tvec3<T>() const
    {
        return { (T)unpack(value.x), (T)unpack(value.y), (T)unpack(value.z) };
    }

    static snorm10x3 zero() { return snorm10x3(float3::zero()); }
    static snorm10x3 one() { return snorm10x3(float3::one()); }
};

template<class T> inline T to(snorm10x3 v) { return (T)v; }


template<class T> inline snorm10x3 encode_tangent(const tvec4<T>& t)
{
    snorm10x3 ret = (tvec3<T>&)t;
    ret.value.w = t.w < 0 ? 1 : 0;
    return ret;
}
inline float4 decode_tangent(snorm10x3 t)
{
    float4 ret;
    (float3&)ret = t;
    ret.w = t.value.w ? -1.0f : 1.0f;
    return ret;
}

// 0.0f - 1.0f in 10 bits x 3
struct unorm10x3
{
    static constexpr float C = float(0x3ff);
    static constexpr float R = 1.0f / float(0x3ff);

    static uint32_t pack(float a)
    {
        return static_cast<uint32_t>(clamp01(a) * C);
    }
    static float unpack(uint32_t a)
    {
        return a * R;
    }

    struct
    {
        uint32_t x : 10;
        uint32_t y : 10;
        uint32_t z : 10;
        uint32_t w : 2;
    } value;

    unorm10x3() {}
    unorm10x3(const unorm10x3& v) : value(v.value) {}
    template<class T> unorm10x3(const tvec3<T>& v) { *this = v; }
    template<class T> void assign(const tvec3<T>& v) { *this = v; }

    template<class T>
    unorm10x3& operator=(const tvec3<T>& v_)
    {
        float3 v = to<float3>(v_);
        (uint32_t&)value = 0;
        value.x = pack(v[0]);
        value.y = pack(v[1]);
        value.z = pack(v[2]);
        return *this;
    }

    template<class T>
    operator tvec3<T>() const
    {
        return { (T)unpack(value.x), (T)unpack(value.y), (T)unpack(value.z) };
    }

    static unorm10x3 zero() { return unorm10x3(float3::zero()); }
    static unorm10x3 one() { return unorm10x3(float3::one()); }
};

template<class T> inline T to(unorm10x3 v) { return (T)v; }

} // namespace mu
