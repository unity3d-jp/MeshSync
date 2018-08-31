#include "pch.h"
#include "msmbUtils.h"

bool IsCamera(FBModel* src)
{
    return src->Is(FBCamera::TypeInfo);
}

bool IsLight(FBModel* src)
{
    return src->Is(FBLight::TypeInfo);
}

bool IsBone(FBModel* src)
{
    return src->Is(FBModelSkeleton::TypeInfo);
}

bool IsMesh(FBModel* src)
{
    return src->ModelVertexData;
    //return !IsCamera(src) && !IsCamera(src) && !IsBone(src) && (
    //    !src->Is(FBModelOptical::TypeInfo) &&
    //    !src->Is(FBModelPath3D::TypeInfo) &&
    //    !src->Is(FBModelMarker::TypeInfo) &&
    //    !src->Is(FBCameraSwitcher::TypeInfo)
    //    );
}


ms::float4x4 to_float4x4(const FBMatrix& src)
{
    auto *m = src.GetData();
    return { {
         (float)m[0][0], (float)m[0][1], (float)m[0][2], (float)m[0][3],
         (float)m[1][0], (float)m[1][1], (float)m[1][2], (float)m[1][3],
         (float)m[2][0], (float)m[2][1], (float)m[2][2], (float)m[2][3],
         (float)m[3][0], (float)m[3][1], (float)m[3][2], (float)m[3][3],
    } };
}

