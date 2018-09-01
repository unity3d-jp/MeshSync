#pragma once

#include "MeshSync/MeshSync.h"

bool IsCamera(FBModel *src);
bool IsLight(FBModel *src);
bool IsBone(FBModel *src);
bool IsMesh(FBModel *src);

std::string GetFilename(const char *src);
std::string GetFilenameWithoutExtension(const char *src);
std::string GetPath(FBModel *src);
std::tuple<double, double> GetTimeRange(FBTake *take);

void EnumerateAllNodes(const std::function<void(FBModel*)>& body);


inline ms::float2 to_float2(const FBVector2<float>& v) { return { v.mValue[0], v.mValue[1] }; }
inline ms::float3 to_float3(const FBVector3<float>& v) { return { v.mValue[0], v.mValue[1], v.mValue[2] }; }
inline ms::float3 to_float3(const FBVector4<float>& v) { return { v.mValue[0], v.mValue[1], v.mValue[2] }; }
inline ms::float3 to_float3(const FBVector3<double>& v) { return { (float)v.mValue[0], (float)v.mValue[1], (float)v.mValue[2] }; }
inline ms::float4 to_float4(const FBColor& v) { return { (float)v.mValue[0], (float)v.mValue[1], (float)v.mValue[2], 1.0f }; }
ms::float4x4 to_float4x4(const FBMatrix& v);

void ABGR2RGBA(char *dst, const char *src, int num);
void ARGB2RGBA(char *dst, const char *src, int num);
void BGRA2RGBA(char *dst, const char *src, int num);
void BGR2RGB(char *dst, const char *src, int num);
