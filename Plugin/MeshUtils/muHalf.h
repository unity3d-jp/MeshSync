#pragma once
#include <cstdint>

namespace mu {

// note: this half doesn't care about Inf nor NaN. simply round down minor bits of exponent and mantissa.
struct half
{
    uint16_t data;

    half() : data(0) {}
    half(const half& v) : data(v.data) {}

    half(float v)
    {
        uint32_t n = (uint32_t&)v;
        uint16_t sign_bit = (n >> 16) & 0x8000;
        uint16_t exponent = (((n >> 23) - 127 + 15) & 0x1f) << 10;
        uint16_t mantissa = (n >> (23 - 10)) & 0x3ff;

        data = sign_bit | exponent | mantissa;
    }

    half& operator=(float v)
    {
        *this = half(v);
        return *this;
    }

    float to_float() const
    {
        uint32_t sign_bit = (data & 0x8000) << 16;
        uint32_t exponent = ((((data >> 10) & 0x1f) - 15 + 127) & 0xff) << 23;
        uint32_t mantissa = (data & 0x3ff) << (23 - 10);

        uint32_t r = sign_bit | exponent | mantissa;
        return (float&)r;
    }
};

struct snorm16
{
    int16_t data;

    snorm16() : data(0) {}
    snorm16(const snorm16& v) : data(v.data) {}
    snorm16(float v) : data(int16_t(v * 32767.0f)) {}

    snorm16& operator=(float v)
    {
        *this = snorm16(v);
        return *this;
    }
    float to_float() const { return (float)data / 32767.0f; }

    static snorm16 zero() { return snorm16(0.0f); }
    static snorm16 one() { return snorm16(1.0f); }
};

struct unorm16
{
    uint16_t data;

    unorm16() : data(0) {}
    unorm16(const unorm16& v) : data(v.data) {}
    unorm16(float v) : data(uint16_t(v * 65535.0f)) {}

    unorm16 & operator=(float v)
    {
        *this = unorm16(v);
        return *this;
    }
    float to_float() const { return (float)data / 65535.0f; }

    static unorm16 zero() { return unorm16(0.0f); }
    static unorm16 one() { return unorm16(1.0f); }
};

} // namespace mu
