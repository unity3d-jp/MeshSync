#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <codecvt>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <future>
#include <mutex>
#include <memory>
#include <cassert>

#pragma warning(push)
#pragma warning(disable:4263 4264)
    #include <fbsdk/fbsdk.h>
#pragma warning(pop)

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#endif