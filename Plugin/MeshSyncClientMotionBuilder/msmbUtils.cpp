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

bool IsTransform(FBModel *src)
{
    return src->Is(FBModelSkeleton::TypeInfo) || src->Is(FBModelNull::TypeInfo);
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

