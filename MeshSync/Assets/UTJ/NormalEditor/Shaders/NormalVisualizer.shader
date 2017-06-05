Shader "Hidden/UTJ/MeshSync/NormalVisualizer" {
    Properties {
        [Enum(UnityEngine.Rendering.CompareFunction)] _ZTest("ZTest", Int) = 4
    }

CGINCLUDE
#include "UnityCG.cginc"

float _VertexSize;
float _NormalSize;
float _TangentSize;
float _BinormalSize;

float4 _VertexColor;
float4 _VertexColor2;
float4 _NormalColor;
float4 _TangentColor;
float4 _BinormalColor;
int _OnlySelected = 0;

float4x4 _Transform;
StructuredBuffer<float3> _Points;
StructuredBuffer<float3> _Normals;
StructuredBuffer<float4> _Tangents;
StructuredBuffer<float> _Selection;

struct appdata
{
    float4 vertex : POSITION;
    float4 uv : TEXCOORD0;
    uint instanceID : SV_InstanceID;
};

struct v2f
{
    float4 vertex : SV_POSITION;
    float4 color : TEXCOORD0;
};

v2f vert_vertices(appdata v)
{
    float3 pos = _Points[v.instanceID];

    float s = _Selection[v.instanceID];
    float4 vertex = v.vertex;
    vertex.xyz *= _VertexSize;
    vertex.xyz *= abs(UnityObjectToViewPos(pos).z);
    vertex.xyz += pos;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = lerp(_VertexColor, _VertexColor2, s);
    return o;
}

v2f vert_normals(appdata v)
{
    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += _Points[v.instanceID] + _Normals[v.instanceID] * v.uv.x * _NormalSize * s;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _NormalColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

v2f vert_tangents(appdata v)
{
    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += _Points[v.instanceID] + _Tangents[v.instanceID].xyz * v.uv.x * _TangentSize * s;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _TangentColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

v2f vert_binormals(appdata v)
{
    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    float3 binormal = normalize(cross(_Normals[v.instanceID], _Tangents[v.instanceID].xyz) * _Tangents[v.instanceID].w);
    vertex.xyz += _Points[v.instanceID] + binormal.xyz * v.uv.x * _BinormalSize * s;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _BinormalColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

half4 frag(v2f v) : SV_Target
{
    return v.color;
}
ENDCG

    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent+100" }
        ZTest[_ZTest]
        ZWrite Off
        Blend SrcAlpha OneMinusSrcAlpha

        // pass 0: visualize vertices
        Pass
        {
            CGPROGRAM
            #pragma vertex vert_vertices
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 1: visualize normals
        Pass
        {
            CGPROGRAM
            #pragma vertex vert_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 2: visualize tangents
        Pass
        {
            CGPROGRAM
            #pragma vertex vert_tangents
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 3: visualize binormals
        Pass
        {
            CGPROGRAM
            #pragma vertex vert_binormals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }    }
}
