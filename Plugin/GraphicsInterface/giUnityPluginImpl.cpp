#include "pch.h"
#include "giInternal.h"
#include "giUnityPluginImpl.h"

#include "PluginAPI/IUnityGraphics.h"
#ifdef giSupportD3D9
    #include <d3d9.h>
    #include "PluginAPI/IUnityGraphicsD3D9.h"
#endif
#ifdef giSupportD3D11
    #include <d3d11.h>
    #include "PluginAPI/IUnityGraphicsD3D11.h"
#endif
#ifdef giSupportD3D12
    #include <d3d12.h>
    #include "PluginAPI/IUnityGraphicsD3D12.h"
#endif

namespace gi {

static IUnityInterfaces* g_unity_interface;

static void UNITY_INTERFACE_API UnityOnGraphicsInterfaceEvent(UnityGfxDeviceEventType eventType)
{
    if (eventType == kUnityGfxDeviceEventInitialize) {
        auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
        auto api = unity_gfx->GetRenderer();

#ifdef giSupportD3D9
        if (api == kUnityGfxRendererD3D9) {
            CreateGraphicsInterface(DeviceType::D3D9, g_unity_interface->Get<IUnityGraphicsD3D9>()->GetDevice());
        }
#endif
#ifdef giSupportD3D11
        if (api == kUnityGfxRendererD3D11) {
            CreateGraphicsInterface(DeviceType::D3D11, g_unity_interface->Get<IUnityGraphicsD3D11>()->GetDevice());
        }
#endif
#ifdef giSupportD3D12
        if (api == kUnityGfxRendererD3D12) {
            CreateGraphicsInterface(DeviceType::D3D12, g_unity_interface->Get<IUnityGraphicsD3D12>()->GetDevice());
        }
#endif
#ifdef giSupportOpenGL
        if (api == kUnityGfxRendererOpenGLCore ||
            api == kUnityGfxRendererOpenGLES20 ||
            api == kUnityGfxRendererOpenGLES30)
        {
            CreateGraphicsInterface(DeviceType::OpenGL, nullptr);
        }
#endif
#ifdef giSupportVulkan
        if (api == kUnityGfxRendererVulkan) // todo
        {
            CreateGraphicsInterface(DeviceType::Vulkan, nullptr);
        }
#endif
    }
    else if (eventType == kUnityGfxDeviceEventShutdown) {
        ReleaseGraphicsInterface();
    }
}


void UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_unity_interface = unityInterfaces;
    g_unity_interface->Get<IUnityGraphics>()->RegisterDeviceEventCallback(UnityOnGraphicsInterfaceEvent);
    UnityOnGraphicsInterfaceEvent(kUnityGfxDeviceEventInitialize);
}

void UnityPluginUnload()
{
    auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
    unity_gfx->UnregisterDeviceEventCallback(UnityOnGraphicsInterfaceEvent);
}

} // namespace gi
