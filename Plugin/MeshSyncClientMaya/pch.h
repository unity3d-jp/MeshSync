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

#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MUintArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MMatrix.h>
#include <maya/MMatrixArray.h>
#include <maya/MQuaternion.h>
#include <maya/MPlugArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MUuid.h>
#include <maya/MItDag.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItGeometry.h>
#include <maya/MFnData.h>
#include <maya/MFnSet.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnSkinCluster.h> 
#include <maya/MFnTransform.h> 
#include <maya/MFnPlugin.h>
#include <maya/MEventMessage.h>
#include <maya/MSceneMessage.h>
#include <maya/MDagMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MPolyMessage.h>
#include <maya/MTimerMessage.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgParser.h>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
    #undef GetMessage
#endif