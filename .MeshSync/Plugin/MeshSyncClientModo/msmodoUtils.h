#pragma once

CLxUser_Item GetParent(CLxUser_Item& obj);
std::string GetName(CLxUser_Item& obj);
std::string GetPath(CLxUser_Item& obj);

inline bool LXTypeMatch(const char *t1, const char *t2)
{
    return t1 && t2 && strcmp(t1, t2) == 0;
}

inline mu::float2 to_float2(LXtFVector2 v)
{
    return (mu::float2&)(v[0]);
}
inline mu::float3 to_float3(LXtFVector v)
{
    return (mu::float3&)(v[0]);
}
inline mu::float4 to_float4(LXtFVector4 v)
{
    return (mu::float4&)(v[0]);
}
inline mu::float3x3 to_float4(LXtFMatrix v)
{
    return (mu::float3x3&)(v[0][0]);
}


