#pragma once

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

#include <ppl.h>

#include "maya/MQuaternion.h"
#include "maya/MEulerRotation.h"
#include "maya/MIntArray.h"
#include "maya/MFloatArray.h"
#include "maya/MStringArray.h"
#include "maya/MObjectArray.h"
#include "maya/MItDag.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MFnMesh.h"
#include "maya/MFnTransform.h"
#include "maya/MFnIkJoint.h"

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else 
    #include <dlfcn.h>
    #ifdef __APPLE__
        #include <mach-o/dyld.h>
    #else
        #include <link.h>
    #endif
#endif
