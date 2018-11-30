Shader "MeshSync/Overlay" {
Properties
{
    [Enum(Normals,0,Tangents,1,UV0,2,UV1,3,UV2,4,UV3,5,Colors,6)] _Type("Attribute", Int) = 6
    [Enum(Local,0,World,1)] _WorldSpace("Coordinate", Int) = 0
    _Scale("Scale", Float) = 1.0
    _Offset("Offset", Float) = 0.0
    [Toggle] _Abs("Abs", Float) = 0.0
}

CGINCLUDE
#include "UnityCG.cginc"
int _Type;
int _WorldSpace;
int _Abs;
float _Scale;
float _Offset;

struct ia_out
{
    float4 vertex : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 color : COLOR;
    float4 uv0 : TEXCOORD0;
    float4 uv1 : TEXCOORD1;
    float4 uv2 : TEXCOORD2;
    float4 uv3 : TEXCOORD3;
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct vs_out
{
    float4 vertex : SV_POSITION;
    float4 color : TEXCOORD0;
};


vs_out vert(ia_out v)
{
    vs_out o;
    o.vertex = UnityObjectToClipPos(v.vertex);

    if (_Type == 0) { // normals
        float3 normal = v.normal.xyz;
        if (_WorldSpace)
            normal = mul(unity_ObjectToWorld, float4(normal.xyz, 0.0)).xyz;
        o.color.rgb = (normal * 0.5 + 0.5) * _Scale + _Offset;
        o.color.a = 1.0;
    }
    else if (_Type == 1) { // tangents
        float3 tangent = v.tangent.xyz * v.tangent.w;
        if (_WorldSpace)
            tangent = mul(unity_ObjectToWorld, float4(tangent.xyz, 0.0)).xyz;
        o.color.rgb = (tangent * 0.5 + 0.5) * _Scale + _Offset;
        o.color.a = 1.0;
    }
    else if (_Type == 2) { // uv0
        o.color = float4(v.uv0.xyz * _Scale + _Offset, 1.0);
    }
    else if (_Type == 3) { // uv1
        o.color = float4(v.uv1.xyz * _Scale + _Offset, 1.0);
    }
    else if (_Type == 4) { // uv2
        o.color = float4(v.uv2.xyz * _Scale + _Offset, 1.0);
    }
    else if (_Type == 5) { // uv3
        o.color = float4(v.uv3.xyz * _Scale + _Offset, 1.0);
    }
    else if (_Type == 6) { // colors
        o.color = float4(v.color.xyz * _Scale + _Offset, v.color.w);
    }

    if (_Abs) {
        o.color = abs(o.color);
    }

    return o;
}

float4 frag(vs_out v) : SV_Target
{
    return v.color;
}

ENDCG

    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent+100" }
        //Blend SrcAlpha OneMinusSrcAlpha
        //ZWrite Off
        Cull Off

        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 3.0
            ENDCG
        }
    }
}
