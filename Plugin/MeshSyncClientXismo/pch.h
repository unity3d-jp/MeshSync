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
#include <fstream>
#include <vector>
#include <map>
#include <functional>
#include <future>
#include <mutex>
#include <memory>
#include <GL/glew.h>
