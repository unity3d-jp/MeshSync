#pragma once

#ifdef _WIN32
    #pragma warning(disable:4996)
    #define NOMINMAX
    #include <windows.h>
    #include <ppl.h>
#else 
    #include <dlfcn.h>
    #ifdef __APPLE__
        #include <mach-o/dyld.h>
    #else
        #include <link.h>
    #endif
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <codecvt>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <future>
#include <mutex>
#include <memory>
#include <locale>
#include <cassert>

#include "MQPlugin.h"
#include "MQBasePlugin.h"
#if MQPLUGIN_VERSION > 0x0400
    #include "MQWidget.h"
#endif
#if MQPLUGIN_VERSION >= 0x0464
    #include "MQBoneManager.h"
#endif

#define MQPluginProduct 0x483ADF11
#define MQPluginID 0xB0CC9999
