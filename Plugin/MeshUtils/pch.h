#pragma once

#ifdef _WIN32
    #pragma warning(disable:4996)
    #define NOMINMAX
    #include <windows.h>
    #include <dbghelp.h>
#else 
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cfloat>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <codecvt>
#include <locale>
#include <type_traits>

#include "muConfig.h"
#ifdef muEnableHalf
    #include <OpenEXR/half.h>
#endif // muEnableHalf

#define muImpl
