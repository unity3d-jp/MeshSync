Shader "Hidden/UTJ/MeshSync/NormalVisualizer" {
    Properties {
        [Enum(UnityEngine.Rendering.CompareFunction)] _ZTest("ZTest", Int) = 4
    }

CGINCLUDE
#include "UnityCG.cginc"

float _Size;
half4 _NormalColor;
half4 _TangentColor;
float4x4 _Transform;
StructuredBuffer<float3> _Points;
StructuredBuffer<float3> _Normals;
StructuredBuffer<float4> _Tangents;

struct appdata
{
    float4 vertex : POSITION;
    UNITY_VERTEX_INPUT_INSTANCE_ID
};

struct v2f
{
    float4 vertex : SV_POSITION;
    float4 color : TEXCOORD0;
};

v2f vert_normals(appdata v)
{
    UNITY_SETUP_INSTANCE_ID(v);

    float4 vertex = v.vertex;
#ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
    vertex.xyz += _Points[unity_InstanceID] + _Normals[unity_InstanceID] * _Size;
#endif
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _NormalColor;
    return o;
}

v2f vert_tangents(appdata v)
{
    UNITY_SETUP_INSTANCE_ID(v);

    float4 vertex = v.vertex;
#ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
    vertex.xyz += _Points[unity_InstanceID] + _Tangents[unity_InstanceID].xyz * _Size;
#endif
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _TangentColor;
    return o;
}

half4 frag(v2f v) : SV_Target
{
    return v.color;
}
ENDCG

    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent" }
        ZTest[_ZTest]
        ZWrite Off
        Blend SrcAlpha OneMinusSrcAlpha

        // pass 0: visualize normals
        Pass
        {
            CGPROGRAM
            #pragma vertex vert_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 1: visualize tangents
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
