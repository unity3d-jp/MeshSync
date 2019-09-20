#pragma once

#ifdef _WIN32
    #define WIN32
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #define _USE_MATH_DEFINES
    #define _CRT_SECURE_NO_WARNINGS
    #define HBOOST_ALL_NO_LIB
    #define SESI_LITTLE_ENDIAN
    #define AMD64
    #define SIZEOF_VOID_P 8
    #define FBX_ENABLED 1
    #define OPENCL_ENABLED 1
    #define OPENVDB_ENABLED 1
    #define MAKING_DSO
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <codecvt>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <regex>
#include <future>
#include <mutex>
#include <memory>
#include <cassert>

// Houdini SDK includes

#include <SOP/SOP_Node.h>
