#pragma once

CLxUser_Item GetParent(CLxUser_Item& obj);
std::string GetName(CLxUser_Item& obj);
std::string GetPath(CLxUser_Item& obj);
std::vector<const char*> GetMapNames(CLxUser_MeshMap& mmap, const LXtID4& id4);

inline bool valid(CLxUser_Item& obj)
{
    // Type() seems return 0 if the object is dead
    return obj.test() && obj.Type() != 0;
}

inline bool match(const char *a, const char *b)
{
    return std::strcmp(a, b) == 0;
}

inline mu::float2 to_float2(const LXtFVector2& v)
{
    return (mu::float2&)(v[0]);
}
inline mu::float3 to_float3(const LXtVector& v)
{
    return { (float)v[0], (float)v[1], (float)v[2] };
}
inline mu::float3 to_float3(const LXtFVector& v)
{
    return (mu::float3&)(v[0]);
}
inline mu::float4 to_float4(const LXtFVector4& v)
{
    return (mu::float4&)(v[0]);
}
inline mu::float3x3 to_float4(const LXtFMatrix& v)
{
    return (mu::float3x3&)(v[0][0]);
}
inline mu::float4x4 to_float4x4(const LXtMatrix& v)
{
    return mu::float4x4{
        (float)v[0][0], (float)v[1][0], (float)v[2][0], 0.0f,
        (float)v[0][1], (float)v[1][1], (float)v[2][1], 0.0f,
        (float)v[0][2], (float)v[1][2], (float)v[2][2], 0.0f,
                  0.0f,           0.0f,           0.0f, 1.0f,
    };
}
inline mu::float4x4 to_float4x4(const LXtMatrix4& v)
{
    return mu::float4x4{
        (float)v[0][0], (float)v[0][1], (float)v[0][2], (float)v[0][3],
        (float)v[1][0], (float)v[1][1], (float)v[1][2], (float)v[1][3],
        (float)v[2][0], (float)v[2][1], (float)v[2][2], (float)v[2][3],
        (float)v[3][0], (float)v[3][1], (float)v[3][2], (float)v[3][3],
    };
}

