#pragma  once

std::string GetPath(MDagPath path);
std::string GetPath(MObject node);
bool IsVisible(MObject node);
MObject GetTransform(MDagPath path);
MObject GetTransform(MObject node);

MObject FindSkinCluster(MObject node);
MObject FindOrigMesh(MObject node);
MObject FindMesh(MObject node);
