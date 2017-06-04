Shader "Hidden/UTJ/MeshSync/VertexVisualizer" {
    Properties {
        [Enum(UnityEngine.Rendering.CompareFunction)] _ZTest("ZTest", Int) = 4
    }

CGINCLUDE
#include "UnityCG.cginc"

float4 _Color;
float _Size;
float4x4 _Transform;
StructuredBuffer<float3> _Points;

struct appdata
{
    float4 vertex : POSITION;
    uint instanceID : SV_InstanceID;
};

struct v2f
{
    float4 vertex : SV_POSITION;
};

v2f vert(appdata v)
{
    float3 pos = _Points[v.instanceID];

    float4 vertex = v.vertex;
    vertex.xyz *= _Size;
    vertex.xyz *= abs(UnityObjectToViewPos(pos).z);
    vertex.xyz += pos;
    vertex = mul(mul(UNITY_MATRIX_VP, _Transform), vertex);

    v2f o;
    o.vertex = vertex;
    return o;
}

half4 frag(v2f v) : SV_Target
{
    return _Color;
}
ENDCG

    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent+100" }
        ZTest [_ZTest]
        ZWrite OFF
        Blend SrcAlpha OneMinusSrcAlpha
        Lighting Off

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }
    }
}
