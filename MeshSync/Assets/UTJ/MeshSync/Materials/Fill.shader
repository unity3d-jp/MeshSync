Shader "MeshSync/Fill" {
Properties
{
    _Color("Color", Color) = (0.0, 0.0, 1.0, 1.0)
}

CGINCLUDE
#include "UnityCG.cginc"
float4 _Color;

struct ia_out
{
    float4 vertex : POSITION;
};

struct vs_out
{
    float4 vertex : SV_POSITION;
};


vs_out vert(ia_out v)
{
    vs_out o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    return o;
}

float4 frag(vs_out v) : SV_Target
{
    return _Color;
}

ENDCG

    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent+100" }
        //Blend SrcAlpha OneMinusSrcAlpha
        ZTest Off
        ZWrite Off

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 3.0
            ENDCG
        }
    }
}
