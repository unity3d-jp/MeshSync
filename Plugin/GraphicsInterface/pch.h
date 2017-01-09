#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>

#ifdef _WIN32
    #define GLEW_STATIC
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <wrl.h>
    #include <d3d9.h>
    #include <d3d11.h>
    #include <d3d12.h>
    #include <GL/glew.h>
    #include <vulkan/vulkan.h>
#endif
