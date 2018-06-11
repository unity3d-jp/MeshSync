#pragma once

#if MAYA_LT

#define MGlobal         ___1I0ll
#define MTypeId         ___0lIIO
#define MStatus         ___l1I00
#define MTime           ___ll0l0
#define MMatrix         ___lIl1O
#define MDistance       ___11llI
#define MSelectionList  ___IlIlI
#define MSyntax         ___1lIll
#define MArgList        ___lI101
#define MArgParser      ___1IO1O
#define MPxCommand      ___IlOl1
#define MAnimControl    ___10lIl
#define MSpace          ___1IOIl
#define MAnimUtil       ___O1IlI

#define MObject             ___1IIOI
#define MObjectArray        ___0O11l
#define MDagPath            ___llOl1
#define MDagPathArray       ___lII0O
#define MPlug               ___l0ll0
#define MPlugArray          ___O0l11
#define MString             ___IOl1I
#define MStringArray        ___lO0lI
#define MIntArray           ___Il11O
#define MPoint              ___1Il1l
#define MPointArray         ___lII01
#define MFloatArray         ___1Oll0
#define MFloatPoint         ___O1011
#define MFloatPointArray    ___lI1OO
#define MFloatVector        ___O11I0
#define MFloatVectorArray   ___O1O0O
#define MColor              ___110lO
#define MColorArray         ___IIIIl

#define MItDag              ___l0l1l
#define MItMeshPolygon      ___IOl1O
#define MItDependencyNodes  ___II0I1
#define MItDependencyGraph  ___0OIOI
#define MItGeometry         ___IOOI1

#define MFn                         ___11lOO
#define MFnBase                     ___1IlO1
#define MFnPlugin                   ___l10lI
#define MFnDagNode                  ___O1l0O
#define MFnDependencyNode           ___IIO1l
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

#define MMessage        ___O01lI
#define MSceneMessage   ___0OIIl
#define MDagMessage     ___l10OI
#define MEventMessage   ___11111
#define MDGMessage      ___10IOl
#define MNodeMessage    ___11IlI


#define kMFnInvalid             MFn::kInvalid
#define kMFnWorld               (MFn::Type)32831
#define kMFnTransform           (MFn::Type)15422
#define kMFnJoint               (MFn::Type)15433
#define kMFnCamera              (MFn::Type)32834
#define kMFnLight               MFn::kLight
#define kMFnPointLight          MFn::kPointLight
#define kMFnSpotLight           MFn::kSpotLight
#define kMFnDirectionalLight    MFn::kDirectionalLight
#define kMFnAreaLight           MFn::kAreaLight
#define kMFnMesh                (MFn::Type)32880

#define kMFnSkinClusterFilter   MFn::kSkinClusterFilter
#define kMFnBlendShape          MFn::kBlendShape
#define kMFnLambert             MFn::kLambert
#define kMFnTweak               MFn::kTweak
#define kMFnPolyTweakUV         MFn::kPolyTweakUV
#define kMFnComponentListData   MFn::kComponentListData
#define kMFnMeshVertComponent   MFn::kMeshVertComponent
#define kMFnPointArrayData      MFn::kPointArrayData

#define kMFnConstraint          MFn::kConstraint
#define kMFnAimConstraint       MFn::kAimConstraint
#define kMFnParentConstraint    MFn::kParentConstraint
#define kMFnPointConstraint     MFn::kPointConstraint
#define kMFnScaleConstraint     MFn::kScaleConstraint

#else // MAYA_LT

#define kMFnInvalid             MFn::kInvalid
#define kMFnWorld               MFn::kWorld
#define kMFnTransform           MFn::kTransform
#define kMFnJoint               MFn::kJoint
#define kMFnCamera              MFn::kCamera
#define kMFnLight               MFn::kLight
#define kMFnPointLight          MFn::kPointLight
#define kMFnSpotLight           MFn::kSpotLight
#define kMFnDirectionalLight    MFn::kDirectionalLight
#define kMFnAreaLight           MFn::kAreaLight
#define kMFnMesh                MFn::kMesh

#define kMFnSkinClusterFilter   MFn::kSkinClusterFilter
#define kMFnBlendShape          MFn::kBlendShape
#define kMFnLambert             MFn::kLambert
#define kMFnTweak               MFn::kTweak
#define kMFnPolyTweakUV         MFn::kPolyTweakUV
#define kMFnComponentListData   MFn::kComponentListData
#define kMFnMeshVertComponent   MFn::kMeshVertComponent
#define kMFnPointArrayData      MFn::kPointArrayData

#define kMFnConstraint          MFn::kConstraint
#define kMFnAimConstraint       MFn::kAimConstraint
#define kMFnParentConstraint    MFn::kParentConstraint
#define kMFnPointConstraint     MFn::kPointConstraint
#define kMFnScaleConstraint     MFn::kScaleConstraint


#endif // MAYA_LT
