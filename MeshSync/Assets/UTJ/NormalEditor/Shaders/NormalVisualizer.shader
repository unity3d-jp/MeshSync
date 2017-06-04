Shader "Hidden/UTJ/MeshSync/NormalVisualizer" {
    Properties {
        [Enum(UnityEngine.Rendering.CompareFunction)] _ZTest("ZTest", Int) = 4
    }

CGINCLUDE
#include "UnityCG.cginc"

float _VertexSize;
float _NormalSize;
float _TangentSize;
float4 _VertexColor;
float4 _VertexColor2;
float4 _NormalColor;
float4 _TangentColor;
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

    float4 vertex = v.vertex;
    vertex.xyz *= _VertexSize;
    vertex.xyz *= abs(UnityObjectToViewPos(pos).z);
    vertex.xyz += pos;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = lerp(_VertexColor, _VertexColor2, _Selection[v.instanceID]);
    return o;
}

v2f vert_normals(appdata v)
{
    float4 vertex = v.vertex;
    vertex.xyz += _Points[v.instanceID] + _Normals[v.instanceID] * v.uv.x * _NormalSize;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _NormalColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

v2f vert_tangents(appdata v)
{
    UNITY_SETUP_INSTANCE_ID(v);

    float4 vertex = v.vertex;
    vertex.xyz += _Points[v.instanceID] + _Tangents[v.instanceID].xyz * v.uv.x * _TangentSize;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _TangentColor;
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
    }
}
