#include "pch.h"
#include "msmbUtils.h"

bool IsCamera(FBModel *src)
{
    return src->Is(FBCamera::TypeInfo);
}

bool IsLight(FBModel *src)
{
    return src->Is(FBLight::TypeInfo);
}

bool IsBone(FBModel *src)
{
    return src->Is(FBModelSkeleton::TypeInfo);
}

bool IsMesh(FBModel* src)
{
    return src->ModelVertexData;
}

std::string GetPath(FBModel *src)
{
    std::string ret;
    if (src->Parent)
        ret = GetPath(src->Parent);
    ret += '/';
    ret += src->LongName;
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
    EnumerateAllNodesImpl(FBSystem::TheOne().RootModel, body);
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

