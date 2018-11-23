#pragma once
#include <cstdint>
#include "muMath.h"

namespace mu {

// note: this half doesn't care about Inf nor NaN. simply round down minor bits of exponent and mantissa.
struct half
{
    uint16_t value;

    half() {}
    half(const half& v) : value(v.value) {}

    half(float v)
    {
        uint32_t n = (uint32_t&)v;
        uint16_t sign_bit = (n >> 16) & 0x8000;
        uint16_t exponent = (std::max<int>((n >> 23) - 127 + 15, 0) & 0x1f) << 10;
        uint16_t mantissa = (n >> (23 - 10)) & 0x3ff;

        value = sign_bit | exponent | mantissa;
    }

    half& operator=(float v)
    {
        *this = half(v);
        return *this;
    }

    operator float() const
    {
        uint32_t sign_bit = (value & 0x8000) << 16;
        uint32_t exponent = ((((value >> 10) & 0x1f) - 15 + 127) & 0xff) << 23;
        uint32_t mantissa = (value & 0x3ff) << (23 - 10);

        uint32_t r = sign_bit | exponent | mantissa;
        return (float&)r;
    }
};

// -1.0f - 1.0f <-> -127 - 127
struct snorm8
{
    int8_t value;

    snorm8() {}
    snorm8(const snorm8& v) : value(v.value) {}
    snorm8(float v) : value(int8_t(clamp11(v) * 127.0f)) {}

    snorm8& operator=(float v)
    {
        *this = snorm8(v);
        return *this;
    }
    operator float() const { return (float)value / 127.0f; }

    static snorm8 zero() { return snorm8(0.0f); }
    static snorm8 one() { return snorm8(1.0f); }
};

// 0.0f - 1.0f <-> 0 - 255
struct unorm8
{
    uint8_t value;

    unorm8() {}
    unorm8(const unorm8& v) : value(v.value) {}
    unorm8(float v) : value(uint8_t(clamp01(v) * 255.0f)) {}

    unorm8& operator=(float v)
    {
        *this = unorm8(v);
        return *this;
    }
    operator float() const { return (float)value / 255.0f; }

    static unorm8 zero() { return unorm8(0.0f); }
    static unorm8 one() { return unorm8(1.0f); }
};

// -1.0f - 1.0f <-> 0 - 255
struct unorm8n
{
    uint8_t value;

    unorm8n() {}
    unorm8n(const unorm8n& v) : value(v.value) {}
    unorm8n(float v) : value(uint8_t((clamp11(v) * 0.5f + 0.5f) * 255.0f)) {}

    unorm8n& operator=(float v)
    {
        *this = unorm8n(v);
        return *this;
    }
    operator float() const { return (float)value / 255.0f * 2.0f - 1.0f; }

    static unorm8n zero() { return unorm8n(0.0f); }
    static unorm8n one() { return unorm8n(1.0f); }
};

// -1.0f - 1.0f <-> -32767 - 32767
struct snorm16
{
    int16_t value;

    snorm16() {}
    snorm16(const snorm16& v) : value(v.value) {}
    snorm16(float v) : value(int16_t(clamp11(v) * 32767.0f)) {}

    snorm16& operator=(float v)
    {
        *this = snorm16(v);
        return *this;
    }
    operator float() const { return (float)value / 32767.0f; }

    static snorm16 zero() { return snorm16(0.0f); }
    static snorm16 one() { return snorm16(1.0f); }
};

// 0.0f - 1.0f <-> 0 - 65535
struct unorm16
{
    uint16_t value;

    unorm16() {}
    unorm16(const unorm16& v) : value(v.value) {}
    unorm16(float v) : value(uint16_t(clamp01(v) * 65535.0f)) {}

    unorm16& operator=(float v)
    {
        *this = unorm16(v);
        return *this;
    }
    operator float() const { return (float)value / 65535.0f; }

    static unorm16 zero() { return unorm16(0.0f); }
    static unorm16 one() { return unorm16(1.0f); }
};

using half2 = tvec2<half>;
using half3 = tvec3<half>;
using half4 = tvec4<half>;
using quath = tquat<half>;
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

} // namespace mu
