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

#include "MeshSync/msClient.h"

#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MSceneMessage.h>
#include <maya/MFnData.h>
#include <maya/MItDag.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnPlugin.h>

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
