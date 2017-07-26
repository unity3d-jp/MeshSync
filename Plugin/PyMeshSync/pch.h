#pragma once

#ifdef _WIN32
    #pragma warning(disable:4996)
    #define NOMINMAX
    #include <winsock2.h>
    #include <windows.h>
    #include <amp.h>
    #include <amp_graphics.h>
    #include <amp_math.h>
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
#include <cstdint>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <iostream>
#include <sstream>
#include <numeric>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <thread>

#include "PyBind11/pybind11.h"
#include "PyBind11/operators.h"
#include "PyBind11/stl.h"
