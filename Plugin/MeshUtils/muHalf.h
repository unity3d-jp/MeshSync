#pragma once
#include <cstdint>

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

    static half zero() { return half(0.0f); }
    static half one() { return half(1.0f); }
};


float clamp01(float v);
float clamp11(float v);
double clamp11(double v);

// -1.0f - 1.0f <-> -127 - 127
struct snorm8
{
    static constexpr float C = float(0x7f);
    static constexpr float R = 1.0f / float(0x7f);

    int8_t value;

    snorm8() {}
    snorm8(const snorm8& v) : value(v.value) {}
    snorm8(float v) : value(int8_t(clamp11(v) * C)) {}

    snorm8& operator=(float v)
    {
        *this = snorm8(v);
        return *this;
    }
    operator float() const { return (float)value * R; }

    static snorm8 zero() { return snorm8(0.0f); }
    static snorm8 one() { return snorm8(1.0f); }
};

// 0.0f - 1.0f <-> 0 - 255
struct unorm8
{
    static constexpr float C = float(0xff);
    static constexpr float R = 1.0f / float(0xff);

    uint8_t value;

    unorm8() {}
    unorm8(const unorm8& v) : value(v.value) {}
    unorm8(float v) : value(uint8_t(clamp01(v) * C)) {}

    unorm8& operator=(float v)
    {
        *this = unorm8(v);
        return *this;
    }
    operator float() const { return (float)value * R; }

    static unorm8 zero() { return unorm8(0.0f); }
    static unorm8 one() { return unorm8(1.0f); }
};

// -1.0f - 1.0f <-> 0 - 255
// for audio sample
struct unorm8n
{
    static constexpr float C = float(0xff);
    static constexpr float R = 1.0f / float(0xff);

    uint8_t value;

    unorm8n() {}
    unorm8n(const unorm8n& v) : value(v.value) {}
    unorm8n(float v) : value(uint8_t((clamp11(v) * 0.5f + 0.5f) * C)) {}

    unorm8n& operator=(float v)
    {
        *this = unorm8n(v);
        return *this;
    }
    operator float() const { return (float)value * R * 2.0f - 1.0f; }

    static unorm8n zero() { return unorm8n(0.0f); }
    static unorm8n one() { return unorm8n(1.0f); }
};

// -1.0f - 1.0f <-> -32767 - 32767
struct snorm16
{
    static constexpr float C = float(0x7fff);
    static constexpr float R = 1.0f / float(0x7fff);

    int16_t value;

    snorm16() {}
    snorm16(const snorm16& v) : value(v.value) {}
    snorm16(float v) : value(int16_t(clamp11(v) * C)) {}

    snorm16& operator=(float v)
    {
        *this = snorm16(v);
        return *this;
    }
    operator float() const { return (float)value * R; }

    static snorm16 zero() { return snorm16(0.0f); }
    static snorm16 one() { return snorm16(1.0f); }
};

// 0.0f - 1.0f <-> 0 - 65535
struct unorm16
{
    static constexpr float C = float(0xffff);
    static constexpr float R = 1.0f / float(0xffff);

    uint16_t value;

    unorm16() {}
    unorm16(const unorm16& v) : value(v.value) {}
    unorm16(float v) : value(uint16_t(clamp01(v) * C)) {}

    unorm16& operator=(float v)
    {
        *this = unorm16(v);
        return *this;
    }
    operator float() const { return (float)value * R; }

    static unorm16 zero() { return unorm16(0.0f); }
    static unorm16 one() { return unorm16(1.0f); }
};

// -1.0f - 1.0f <-> -2147483647 - 2147483647
// for audio sample
struct snorm24
{
    static constexpr double C = double(0x7fffffff);
    static constexpr double R = 1.0 / double(0x7fffffff);

    uint8_t value[3];

    snorm24() {}
    snorm24(const snorm24& v)
    {
        for (int i = 0; i < 3; ++i)
            value[i] = v.value[i];
    }
    snorm24(float v)
    {
        // store upper 24 bits
        int i32 = int((double)clamp11(v) * C);
        value[0] = uint8_t((i32 & 0x0000ff00) >> 8 );
        value[1] = uint8_t((i32 & 0x00ff0000) >> 16);
        value[2] = uint8_t((i32 & 0xff000000) >> 24);
    }

    snorm24& operator=(float v)
    {
        *this = snorm24(v);
        return *this;
    }
    operator float() const
    {
        int i32 = (value[0] << 8) | (value[1] << 16) | (value[2] << 24);
        return float((double)i32 * R);
    }

    static snorm24 zero() { return snorm24(0.0f); }
    static snorm24 one() { return snorm24(1.0f); }
};

// -1.0f - 1.0f <-> -2147483647 - 2147483647
// for audio sample
struct snorm32
{
    static constexpr double C = double(0x7fffffff);
    static constexpr double R = 1.0 / double(0x7fffffff);

    int value;

    snorm32() {}
    snorm32(const snorm32& v) : value(v.value) {}
    snorm32(float v) : value(int((double)clamp11(v) * C)) {}

    snorm32& operator=(float v)
    {
        *this = snorm32(v);
        return *this;
    }
    operator float() const { return float((double)value * R); }

    static snorm32 zero() { return snorm32(0.0f); }
    static snorm32 one() { return snorm32(1.0f); }
};

template<class T> inline T to(float v) { return (T)v; }

} // namespace mu
