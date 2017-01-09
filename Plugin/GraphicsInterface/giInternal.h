#pragma  once

#ifndef gdLog
    #define giLog(...)
    #define giLogError(...)
#endif

#ifdef _WIN32
    #define giSupportD3D9
    #define giSupportD3D11
    #define giSupportD3D12
    #define giSupportOpenGL
    //#define giSupportVulkan
#else
    #define giSupportOpenGL
#endif

#include "GraphicsInterface.h"

namespace gi {

class GraphicsInterface;
GraphicsInterface* CreateGraphicsInterfaceD3D9(void *device);
GraphicsInterface* CreateGraphicsInterfaceD3D11(void *device);
GraphicsInterface* CreateGraphicsInterfaceD3D12(void *device);
GraphicsInterface* CreateGraphicsInterfaceOpenGL(void *device);
GraphicsInterface* CreateGraphicsInterfaceVulkan(void *device);


// i.e:
//  roundup<16>(31) : 32
//  roundup<16>(32) : 32
//  roundup<16>(33) : 48
template<int N, class IntType>
inline IntType roundup(IntType v)
{
    return v + (N - v % N);
}

// i.e: 
//  ceildiv(31, 16) : 2
//  ceildiv(32, 16) : 2
//  ceildiv(33, 16) : 3
template<class IntType>
inline IntType ceildiv(IntType a, IntType b)
{
    return a / b + (a%b == 0 ? 0 : 1);
}

static inline bool operator&(ResourceFlags a, ResourceFlags b) { return ((int)a & (int)b) != 0; }


inline void CopyRegion(void *dst, int dst_pitch, const void *src, int src_pitch, int num_rows)
{
    if (dst_pitch == src_pitch) {
        memcpy(dst, src, dst_pitch * num_rows);
    }
    else {
        auto *tdst = (char*)dst;
        auto *tsrc = (const char*)src;
        int copy_size = std::min<int>(dst_pitch, src_pitch);
        for (int ri = 0; ri < num_rows; ++ri) {
            memcpy(tdst, tsrc, copy_size);
            tdst += dst_pitch;
            tsrc += src_pitch;
        }
    }
}

#ifdef _WIN32
DXGI_FORMAT GetDXGIFormat(TextureFormat fmt);
Result TranslateReturnCode(HRESULT hr);
#endif

} // namespace gi
