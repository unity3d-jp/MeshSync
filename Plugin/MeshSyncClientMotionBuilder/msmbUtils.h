#pragma once

#include "MeshSync/MeshSync.h"

struct ClusterScope
{
    FBCluster *cluster;
    ClusterScope(FBCluster *c, int li) : cluster(c) { cluster->ClusterBegin(li); }
    ~ClusterScope() { cluster->ClusterEnd(); }
};

struct GeometryScope
{
    FBGeometry *geom;
    GeometryScope(FBGeometry *g) : geom(g) { geom->GeometryBegin(); }
    ~GeometryScope() { geom->GeometryEnd(); }
};

bool IsCamera(FBModel *src);
bool IsLight(FBModel *src);
bool IsBone(FBModel *src);
bool IsMesh(FBModel *src);

const char* GetName(FBModel *src);
std::string GetPath(FBModel *src);
std::tuple<double, double> GetTimeRange(FBTake *take);

void EnumerateAllNodes(const std::function<void(FBModel*)>& body);

void DbgPrintCluster(FBModel *model);

inline ms::float2 to_float2(const FBVector2<float>& v) { return { v.mValue[0], v.mValue[1] }; }
inline ms::float3 to_float3(const FBVector3<float>& v) { return { v.mValue[0], v.mValue[1], v.mValue[2] }; }
inline ms::float3 to_float3(const FBVector4<float>& v) { return { v.mValue[0], v.mValue[1], v.mValue[2] }; }
inline ms::float3 to_float3(const FBVector3<double>& v) { return { (float)v.mValue[0], (float)v.mValue[1], (float)v.mValue[2] }; }
inline ms::float4 to_float4(const FBColor& v) { return { (float)v.mValue[0], (float)v.mValue[1], (float)v.mValue[2], 1.0f }; }
ms::float4x4 to_float4x4(const FBMatrix& v);

// T: property list
template<class T, class Body>
void Each(T& list, const Body& body)
{
    int n = list.GetCount();
    for (int i = 0; i < n; ++i)
        body(list[i]);
}
