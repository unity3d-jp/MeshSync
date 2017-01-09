#include "pch.h"
#include "giInternal.h"

namespace gi {

GraphicsInterface::~GraphicsInterface()
{
}


void GraphicsInterface::releaseStagingResource(MapContext& ctx)
{
    if (ctx.keep_staging_resource && ctx.staging_resource) {
        releaseBuffer(ctx.staging_resource);
        ctx.staging_resource = nullptr;
    }
}

Result GraphicsInterface::readBuffer(void *dst_mem, void *src_buf, size_t read_size, BufferType type)
{
    MapContext ctx;
    ctx.resource = src_buf;
    ctx.type = type;
    ctx.mode = MapMode::Read;

    auto res = mapBuffer(ctx);
    if (res == Result::OK) {
        memcpy(dst_mem, ctx.data_ptr, read_size);
        res = unmapBuffer(ctx);
    }
    return res;
}


Result GraphicsInterface::writeBuffer(void *dst_buf, const void *src_mem, size_t write_size, BufferType type)
{
    MapContext ctx;
    ctx.resource = dst_buf;
    ctx.type = type;
    ctx.mode = MapMode::Write;

    auto res = mapBuffer(ctx);
    if (res == Result::OK) {
        memcpy(ctx.data_ptr, src_mem, write_size);
        res = unmapBuffer(ctx);
    }
    return res;
}

int GraphicsInterface::GetTexelSize(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBAu8:  return 4;
    case TextureFormat::RGu8:    return 2;
    case TextureFormat::Ru8:     return 1;

    case TextureFormat::RGBAf16:
    case TextureFormat::RGBAi16: return 8;
    case TextureFormat::RGf16:
    case TextureFormat::RGi16:   return 4;
    case TextureFormat::Rf16:
    case TextureFormat::Ri16:    return 2;

    case TextureFormat::RGBAf32:
    case TextureFormat::RGBAi32: return 16;
    case TextureFormat::RGf32:
    case TextureFormat::RGi32:   return 8;
    case TextureFormat::Rf32:
    case TextureFormat::Ri32:    return 4;
    }
    return 0;
}


static GraphicsInterface *g_gfx_device;

GraphicsInterface* CreateGraphicsInterface(DeviceType type, void *device_ptr)
{
    switch (type) {
#ifdef giSupportD3D9
    case DeviceType::D3D9:
        g_gfx_device = CreateGraphicsInterfaceD3D9(device_ptr);
        break;
#endif
#ifdef giSupportD3D11
    case DeviceType::D3D11:
        g_gfx_device = CreateGraphicsInterfaceD3D11(device_ptr);
        break;
#endif
#ifdef giSupportD3D12
    case DeviceType::D3D12:
        g_gfx_device = CreateGraphicsInterfaceD3D12(device_ptr);
        break;
#endif
#ifdef giSupportOpenGL
    case DeviceType::OpenGL:
        g_gfx_device = CreateGraphicsInterfaceOpenGL(device_ptr);
        break;
#endif
#ifdef giSupportVulkan
    case DeviceType::Vulkan:
        g_gfx_device = CreateGraphicsInterfaceVulkan(device_ptr);
        break;
#endif
    }
    return g_gfx_device;
}

GraphicsInterface* GetGraphicsInterface()
{
    return g_gfx_device;
}

void ReleaseGraphicsInterface()
{
    if (g_gfx_device) {
        delete g_gfx_device;
        g_gfx_device = nullptr;
    }
}

} // namespace gi
