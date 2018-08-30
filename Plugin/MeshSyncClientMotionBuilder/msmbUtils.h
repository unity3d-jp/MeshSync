#pragma once

inline ms::float2 to_float2(const FBVector2<float>& v)
{
    return { v.mValue[0], v.mValue[1] };
}

inline ms::float3 to_float3(const FBVector3<float>& v)
{
    return { v.mValue[0], v.mValue[1], v.mValue[2] };
}

inline ms::float3 to_float3(const FBVector4<float>& v)
{
    return { v.mValue[0], v.mValue[1], v.mValue[2] };
}

ms::float4x4 to_float4x4(const FBMatrix& v);
