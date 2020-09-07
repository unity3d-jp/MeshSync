
Shader "MultiUVTest/8UV" {
Properties 	{
	_Color ( "Color", Color) = (1, 1, 1, 1)
}

SubShader {
	Tags { "RenderType"="Opaque" }
	pass {		
		Tags { "LightMode"="ForwardBase"}
		cull back

		CGPROGRAM

		#pragma target 3.0
		#pragma fragmentoption ARB_precision_hint_fastest           

		#pragma vertex EightUV_VS
		#pragma fragment EightUV_PS

        #include "UnityCG.cginc"

        float4 _Color;

		struct VS_IN {
		    float4 vertex : POSITION;
		    float3 normal : NORMAL;
		    float4 texcoord0 : TEXCOORD0;
		    float4 texcoord1 : TEXCOORD1;
		    float4 texcoord2 : TEXCOORD2;
		    float4 texcoord3 : TEXCOORD3;
		    float4 texcoord4 : TEXCOORD4;
		    float4 texcoord5 : TEXCOORD5;
		    float4 texcoord6 : TEXCOORD6;
		    float4 texcoord7 : TEXCOORD7;
		    UNITY_VERTEX_INPUT_INSTANCE_ID
		};

		
        struct PS_IN  {
            float4 pos : SV_POSITION;
        };

//----------------------------------------------------------------------------------------------------------------------
        PS_IN EightUV_VS(VS_IN v) {
            PS_IN o;
            o.pos = UnityObjectToClipPos(v.vertex);
            return o;
        }


        float4 EightUV_PS(PS_IN input) : COLOR {
            return float4(_Color);
        }

		ENDCG
	}		

}
FallBack "Diffuse"

} //end Shader
