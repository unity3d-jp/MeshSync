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

#ifdef _WIN32
    #define msMaya_UsePPL
#else 
    #define msMaya_UseTBB
#endif

#ifdef msMaya_UsePPL
    #include <ppl.h>
#endif
#ifdef msMaya_UseTBB
    #include <tbb/tbb.h>
    namespace concurrency = tbb;
#endif

// this must be before maya includes
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"

// avoid multiple definition of `MApiVersion'
//#define _MApiVersion

// avoid redefinition of bool on maya 2015
#define _BOOL

#include "MayaLTSupport.h"

#include <maya/MAnimControl.h>
#include <maya/MGlobal.h>
#include <maya/MDistance.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MDagMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MUintArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MMatrixArray.h>
#include <maya/MQuaternion.h>
#include <maya/MPlugArray.h>
#include <maya/MItDag.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItGeometry.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnData.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnSet.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnTransform.h>
#include <maya/MFnCamera.h>
#include <maya/MFnLight.h>
#include <maya/MFnAmbientLight.h>
#include <maya/MFnAreaLight.h>
#include <maya/MFnDirectionalLight.h>
#include <maya/MFnPointLight.h>
#include <maya/MFnSpotLight.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnVolumeLight.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MEventMessage.h>
#include <maya/MSceneMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MPolyMessage.h>
#include <maya/MTimerMessage.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgParser.h>
#include <maya/MAnimUtil.h>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#endif