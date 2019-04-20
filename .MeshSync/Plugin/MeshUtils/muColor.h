#pragma once
#include "muMath.h"
#include "muHalf.h"
#include "muRawVector.h"

namespace mu {

inline float4 Color32ToFloat4(uint32_t c)
{
    return{
        (float)(c & 0xff) / 255.0f,
        (float)((c & 0xff00) >> 8) / 255.0f,
        (float)((c & 0xff0000) >> 16) / 255.0f,
        (float)(c >> 24) / 255.0f,
    };
}
inline uint32_t Float4ToColor32(const float4& c)
{
    return
        (int)(c.x * 255.0f) |
        ((int)(c.y * 255.0f) << 8) |
        ((int)(c.z * 255.0f) << 16) |
        ((int)(c.w * 255.0f) << 24);
}

template<class T>
inline void ABGR2RGBA(T *dst, const T *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        auto t = src[pi];
        dst[pi] = { t[3], t[2], t[1], t[0] };
    }
}

template<class T>
inline void ARGB2RGBA(T *dst, const T *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        auto t = src[pi];
        dst[pi] = { t[1], t[2], t[3], t[0] };
    }
}

template<class T>
inline void BGRA2RGBA(T *dst, const T *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        auto t = src[pi];
        dst[pi] = { t[2], t[1], t[0], t[3] };
    }
}

template<class T>
inline void BGR2RGB(T *dst, const T *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        auto t = src[pi];
        dst[pi] = { t[2], t[1], t[0] };
    }
}

template<class RG, class RGB>
inline void RG2RGB(RGB *dst, const RG *src, int num)
{
    for (int pi = 0; pi < num; ++pi) {
        auto t = src[pi];
        dst[pi] = { t[0], t[1], 0 };
    }
}
template<class RG, class RGB>
inline void RG2RGB(RawVector<char>& src)
{
    size_t num = src.size() / sizeof(RG);

    RawVector<char> tmp;
    tmp.resize_discard(num * sizeof(RGB));
    RG2RGB((RGB*)tmp.data(), (const RG*)src.data(), (int)num);
    src.swap(tmp);
}

} // namespace mu
