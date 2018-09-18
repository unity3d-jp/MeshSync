#pragma once
#include "muMath.h"
#include "muHalf.h"

namespace mu {

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

} // namespace mu
