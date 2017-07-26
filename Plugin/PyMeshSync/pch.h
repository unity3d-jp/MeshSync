#pragma once

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
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
