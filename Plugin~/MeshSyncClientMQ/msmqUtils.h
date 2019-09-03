#pragma once

#include "MeshSync/MeshSync.h"

#define MaxNameBuffer 128

using namespace mu;

inline float4 to_float4(const MQColor& v)
{
    return { v.r, v.g, v.b, 1.0f };
}
inline float3 to_float3(const MQPoint& v)
{
    return (const float3&)v;
}
inline float4x4 to_float4x4(const MQMatrix& v)
{
    return (const float4x4&)v;
}
inline MQColor to_MQColor(const float4& v)
{
    return MQColor(v[0], v[1], v[2]);
}

std::string GetName(MQObject obj);
std::string GetName(MQMaterial obj);
std::string GetPath(MQDocument doc, MQObject obj);
bool ExtractID(const char *name, int& id);

float3 ToEular(const MQAngle& ang, bool flip_head = false);
quatf ToQuaternion(const MQAngle& ang);
void ExtractLocalTransform(MQObject obj, float3& pos, quatf& rot, float3& scale);
float4x4 ExtractGlobalMatrix(MQDocument doc, MQObject obj);
