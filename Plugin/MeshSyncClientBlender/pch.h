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
#include "PyBind11/eval.h"
#include "PyBind11/stl.h"
namespace py = pybind11;


#include "RNA_types.h"
#include "intern/rna_internal_types.h"
#include "DNA_object_types.h"
#include "DNA_material_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_modifier_types.h"
#include "DNA_armature_types.h"
#include "DNA_anim_types.h"
#include "DNA_key_types.h"
#include "intern/bpy_rna.h"
#include "BLI_utildefines.h"
#include "BLI_math_base.h"
#include "BKE_fcurve.h"
