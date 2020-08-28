#pragma once

#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <d3d11.h>
    #include <d3d11_4.h>
    #include <d3d12.h>
    #include <dxgi1_4.h>
    #include <dxgiformat.h>
    #include <dxcapi.h>
    #include <comdef.h>
#endif // _WIN32

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <iostream>
#include <sstream>
#include <thread>
#include <cassert>

