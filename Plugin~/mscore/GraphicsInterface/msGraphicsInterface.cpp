#include "pch.h"
#include "msGraphicsInterface.h"

namespace ms {

GraphicsInterface::~GraphicsInterface()
{
}


static GraphicsInterface *g_gfx_device;


GraphicsInterface* CreateGraphicsInterfaceD3D11(void *device);
GraphicsInterface* CreateGraphicsInterfaceD3D12(void *device);
GraphicsInterface* CreateGraphicsInterfaceOpenGL(void *device);
GraphicsInterface* CreateGraphicsInterfaceVulkan(void *device);
GraphicsInterface* CreateGraphicsInterfaceMetal(void *device);

GraphicsInterface* CreateGraphicsInterface(DeviceType type, void *device_ptr)
{
    switch (type) {
#ifdef msEnableD3D11
    case DeviceType::D3D11:
        g_gfx_device = CreateGraphicsInterfaceD3D11(device_ptr);
        break;
#endif
#ifdef msEnableD3D12
    case DeviceType::D3D12:
        g_gfx_device = CreateGraphicsInterfaceD3D12(device_ptr);
        break;
#endif
#ifdef msEnableOpenGL
    case DeviceType::OpenGL:
        g_gfx_device = CreateGraphicsInterfaceOpenGL(device_ptr);
        break;
#endif
#ifdef msEnableVulkan
    case DeviceType::Vulkan:
        g_gfx_device = CreateGraphicsInterfaceVulkan(device_ptr);
        break;
#endif
#ifdef msEnableMetal
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

static void ReleaseGraphicsInterface()
{
    if (g_gfx_device) {
        delete g_gfx_device;
        g_gfx_device = nullptr;
    }
}

} // namespace ms
