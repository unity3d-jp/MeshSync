#pragma  once

std::string GetPath(MDagPath path);
std::string GetPath(MObject node);

MDagPath GetDagPath(MObject node);
bool IsVisible(MObject node);
MObject GetTransform(MDagPath path);
MObject GetTransform(MObject node);

MObject FindMesh(MObject node);
MObject FindSkinCluster(MObject node);
MObject FindBlendShape(MObject node);
MObject FindOrigMesh(MObject node);
MObject FindInputMesh(const MFnGeometryFilter& gf, const MDagPath& path);
MObject FindOutputMesh(const MFnGeometryFilter& gf, const MDagPath& path);

void DumpPlugInfo(MPlug plug);
