#pragma once

#ifdef _WIN32
    #define WIN32
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #define _USE_MATH_DEFINES
    #define _CRT_SECURE_NO_WARNINGS
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


// Houdini defines & includes
#define HBOOST_ALL_NO_LIB
#define SESI_LITTLE_ENDIAN
#define AMD64
#define SIZEOF_VOID_P 8
//#define FBX_ENABLED 1
//#define OPENCL_ENABLED 1
//#define OPENVDB_ENABLED 1
#define MAKING_DSO

#include <ROP/ROP_Node.h>
#include <ROP/ROP_Error.h>
#include <ROP/ROP_Templates.h>
#include <SOP/SOP_Node.h>
#include <SOP/SOP_Guide.h>
#include <GU/GU_Detail.h>
#include <GT/GT_RefineParms.h>
#include <GA/GA_Iterator.h>
#include <OP/OP_AutoLockInputs.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <UT/UT_Vector3.h>
#include <SYS/SYS_Math.h>
