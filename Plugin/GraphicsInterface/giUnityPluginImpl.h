#pragma once

struct IUnityInterfaces;

namespace gi {
    void UnityPluginLoad(IUnityInterfaces* unityInterfaces);
    void UnityPluginUnload();
} // namespace gi
