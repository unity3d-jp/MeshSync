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

} // namespace mu
