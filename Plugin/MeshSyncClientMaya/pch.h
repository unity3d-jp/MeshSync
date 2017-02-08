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

#ifdef _WIN32
    #define msMaya_UsePPL
#else 
    #define msMaya_UseTBB
#endif

#ifdef msMaya_UsePPL
    #include <ppl.h>
#endif
#ifdef msMaya_UseTBB
    #include <tbb.h>
    namespace concurrency = tbb;
#endif

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
