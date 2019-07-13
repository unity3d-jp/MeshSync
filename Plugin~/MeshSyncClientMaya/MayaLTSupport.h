#pragma once

#if _WIN32
#if MAYA_LT

#define MGlobal                     ___1I0ll
#define MTypeId                     ___0lIIO
#define MStatus                     ___l1I00
#define MTime                       ___ll0l0
#define MVector                     ___0I0IO
#define MQuaternion                 ___11l0l
#define MMatrix                     ___lIl1O
#define MDistance                   ___11llI
#define MSelectionList              ___IlIlI
#define MSyntax                     ___1lIll
#define MArgList                    ___lI101
#define MArgParser                  ___1IO1O
#define MPxCommand                  ___IlOl1
#define MAnimControl                ___10lIl
#define MSpace                      ___1IOIl
#define MAnimUtil                   ___O1IlI

#define MObject                     ___1IIOI
#define MObjectArray                ___0O11l
#define MDagPath                    ___llOl1
#define MDagPathArray               ___lII0O
#define MPlug                       ___l0ll0
#define MPlugArray                  ___O0l11
#define MString                     ___IOl1I
#define MStringArray                ___lO0lI
#define MIntArray                   ___Il11O
#define MPoint                      ___1Il1l
#define MPointArray                 ___lII01
#define MFloatArray                 ___1Oll0
#define MFloatPoint                 ___O1011
#define MFloatPointArray            ___lI1OO
#define MFloatVector                ___O11I0
#define MFloatVectorArray           ___O1O0O
#define MColor                      ___110lO
#define MColorArray                 ___IIIIl

#define MItDag                      ___l0l1l
#define MItMeshPolygon              ___IOl1O
#define MItDependencyNodes          ___II0I1
#define MItDependencyGraph          ___0OIOI
#define MItGeometry                 ___IOOI1

#define MFn                         ___11lOO
#define MFnBase                     ___1IlO1
#define MFnPlugin                   ___l10lI
#define MFnAttribute                ___OOl1I
#define MFnCompoundAttribute        ___O0IIO
#define MFnDagNode                  ___O1l0O
#define MFnDependencyNode           ___IIO1l
#define MFnTransform                ___IIO11
#define MFnCamera                   ___I0lIl
#define MFnLight                    ___IlI1I
#define MFnSpotLight                ___IO0Il
#define MFnDirectionalLight         ___00Il0
#define MFnMatrixData               ___001l0
#define MFnMesh                     ___10IOO
#define MFnSkinCluster              ___I0lO1
#define MFnBlendShapeDeformer       ___OI1lI
#define MFnGeometryFilter           ___O11ll
#define MFnPointArrayData           ___lll01
#define MFnComponentListData        ___OOIl1
#define MFnLambertShader            ___0Illl
#define MFnSingleIndexedComponent   ___1O0ll

#define MMessage                    ___O01lI
#define MSceneMessage               ___0OIIl
#define MDagMessage                 ___l10OI
#define MEventMessage               ___11111
#define MDGMessage                  ___10IOl
#define MNodeMessage                ___11IlI

#endif // MAYA_LT
#endif // _WIN32
