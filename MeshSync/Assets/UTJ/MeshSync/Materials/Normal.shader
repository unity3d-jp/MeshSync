Shader "Unlit/Normal"
{
    Properties
    {
        _Opacity("Opacity", Range(0.0, 1.0)) = 1.0
    }
    SubShader
    {
        Tags{ "RenderType" = "Transparent" "Queue" = "Transparent" }
        Blend SrcAlpha OneMinusSrcAlpha

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc"

            float _Opacity;

            struct appdata
            {
                float4 vertex : POSITION;
                float4 normal : NORMAL;
                float4 tangent : TANGENT;
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float4 normal : TEXCOORD0;
            };
            
            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);

                float3 normal = v.normal.xyz;
                //float3 binormal = normalize(cross(normal, v.tangent.xyz) * v.tangent.w);
                //float3x3 tbn = float3x3(v.tangent.xyz, binormal, normal);
                //normal = normalize(mul(normal, transpose(tbn)));

                o.normal.rgb = normal * 0.5 + 0.5;
                o.normal.a = _Opacity;
                return o;
            }
            
            float4 frag (v2f i) : SV_Target
            {
                return i.normal;
            }
            ENDCG
        }
    }
}
