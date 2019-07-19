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

bool IsNull(FBModel *src);
bool IsCamera(FBModel *src);
bool IsLight(FBModel *src);
bool IsBone(FBModel *src);
bool IsMesh(FBModel *src);
bool IsVisibleInHierarchy(FBModel *src);

const char* GetName(FBModel *src);
std::string GetPath(FBModel *src);
std::tuple<double, double> GetTimeRange(FBTake *take);

void EnumerateAllNodes(const std::function<void(FBModel*)>& body);

inline ms::float2 to_float2(const FBVector2<float>& v) { return { v.mValue[0], v.mValue[1] }; }
inline ms::float3 to_float3(const FBVector3<float>& v) { return { v.mValue[0], v.mValue[1], v.mValue[2] }; }
inline ms::float3 to_float3(const FBVector4<float>& v) { return { v.mValue[0], v.mValue[1], v.mValue[2] }; }
inline ms::float3 to_float3(const FBVector3<double>& v) { return { (float)v.mValue[0], (float)v.mValue[1], (float)v.mValue[2] }; }
inline ms::float4 to_float4(const FBColor& v) { return { (float)v.mValue[0], (float)v.mValue[1], (float)v.mValue[2], 1.0f }; }
inline ms::float4x4 to_float4x4(const FBMatrix& src)
{
    auto m = (const double*)&src;
    return { {
         (float)m[0], (float)m[1], (float)m[2], (float)m[3],
         (float)m[4], (float)m[5], (float)m[6], (float)m[7],
         (float)m[8], (float)m[9], (float)m[10], (float)m[11],
         (float)m[12], (float)m[13], (float)m[14], (float)m[15],
    } };
}


// T: property list
template<class T, class Body>
inline void Each(T& list, const Body& body)
{
    int n = list.GetCount();
    for (int i = 0; i < n; ++i)
        body(list[i]);
}

template<class Body>
inline void EachBones(FBModel *model, const Body& body)
{
    if (FBCluster *cluster = model->Cluster) {
        int num_links = cluster->LinkGetCount();
        for (int li = 0; li < num_links; ++li) {
            auto bone = cluster->LinkGetModel(li);
            body(bone);
        }
    }
}

#ifdef mscDebug
void DbgPrintProperties(FBPropertyManager& properties);
void DbgPrintAnimationNode(FBAnimationNode *node);
void DbgPrintCluster(FBModel *model);
#else
#define DbgPrintProperties(...)
#define DbgPrintAnimationNode(...);
#define DbgPrintCluster(...);
#endif
