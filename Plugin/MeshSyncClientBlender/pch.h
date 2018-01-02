#pragma once

#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
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
#include <mutex>
#include <atomic>
#include <algorithm>
#include <thread>
#include <future>

#include "PyBind11/pybind11.h"
#include "PyBind11/operators.h"
#include "PyBind11/stl.h"
namespace py = pybind11;


#include "blender/makesrna/RNA_types.h"
#include "blender/makesdna/DNA_object_types.h"
#include "blender/makesdna/DNA_mesh_types.h"
#include "blender/makesdna/DNA_meshdata_types.h"
#include "blender/python/intern/bpy_rna.h"
