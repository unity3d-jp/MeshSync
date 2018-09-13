#include "pch.h"
#include "msmbUtils.h"

bool IsCamera(FBModel *src)
{
    return src && src->Is(FBCamera::TypeInfo);
}

bool IsLight(FBModel *src)
{
    return src && src->Is(FBLight::TypeInfo);
}

bool IsBone(FBModel *src)
{
    return src && src->Is(FBModelSkeleton::TypeInfo);
}

bool IsMesh(FBModel* src)
{
    return src && src->ModelVertexData;
}

const char* GetName(FBModel *src)
{
    return src->LongName;
}

std::string GetPath(FBModel *src)
{
    std::string ret;
    if (src->Parent)
        ret = GetPath(src->Parent);
    ret += '/';
    ret += GetName(src);
    return ret;
}

std::tuple<double, double> GetTimeRange(FBTake *take)
{
    FBTimeSpan timespan = take->LocalTimeSpan;
    return { timespan.GetStart().GetSecondDouble(), timespan.GetStop().GetSecondDouble() };
}

static void EnumerateAllNodesImpl(FBModel *node, const std::function<void(FBModel*)>& body)
{
    body(node);

    int num_children = node->Children.GetCount();
    for (int i = 0; i < num_children; i++)
        EnumerateAllNodesImpl(node->Children[i], body);
}
void EnumerateAllNodes(const std::function<void(FBModel*)>& body)
{
    auto& scene = FBSystem::TheOne().Scene;
    Each(scene->Cameras, [&body](FBCamera *cam) {
        if (cam->LongName == "Producer Perspective")
            body(cam);
    });
    EnumerateAllNodesImpl(scene->RootModel, body);
}


void DbgPrintCluster(FBModel *model)
{
    FBCluster *cluster = model->Cluster;
    if (!cluster)
        return;

    FBModelVertexData *vd = model->ModelVertexData;
    RawVector<float> total_weights;
    total_weights.resize_zeroclear(vd->GetVertexCount());

    mu::Print("Cluster %s:\n", cluster->LongName.AsString());
    int num_links = cluster->LinkGetCount();
    for (int li = 0; li < num_links; ++li) {
        ClusterScope scope(cluster, li);

        auto bone = cluster->LinkGetModel(li);
        mu::Print("  Bone %s:\n", bone->LongName.AsString());

        // weights
        int n = cluster->VertexGetCount();
        mu::Print("    %d vertices:\n", n);
        for (int vi = 0; vi < n; ++vi) {
            int i = cluster->VertexGetNumber(vi);
            float w = (float)cluster->VertexGetWeight(vi);
            mu::Print("      %d - %f:\n", i, w);
            total_weights[i] += w;
        }
    }

    mu::Print("  Weights %s:\n", cluster->LongName.AsString());
    for (float v : total_weights) {
        mu::Print("    %f\n", v);
    }
}


ms::float4x4 to_float4x4(const FBMatrix& src)
{
    auto m = (const double*)&src;
    return { {
         (float)m[0], (float)m[1], (float)m[2], (float)m[3],
         (float)m[4], (float)m[5], (float)m[6], (float)m[7],
         (float)m[8], (float)m[9], (float)m[10], (float)m[11],
         (float)m[12], (float)m[13], (float)m[14], (float)m[15],
    } };
}
