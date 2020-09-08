
Shader "MultiUVTest/8UV" {
Properties 	{
	_Color0 ( "Color 0", Color) = (1, 1, 1, 1)
	_TexUV0 ("Texture using UV0", 2D) = "black" {}
	_Color1 ( "Color 1", Color) = (1, 1, 1, 1)
	_TexUV1 ("Texture using UV1", 2D) = "black" {}
	_Color2 ( "Color 2", Color) = (1, 1, 1, 1)
	_TexUV2 ("Texture using UV2", 2D) = "black" {}
	_Color3 ( "Color 3", Color) = (1, 1, 1, 1)
	_TexUV3 ("Texture using UV3", 2D) = "black" {}
	_Color4 ( "Color 4", Color) = (1, 1, 1, 1)
	_TexUV4 ("Texture using UV4", 2D) = "black" {}
	_Color5 ( "Color 5", Color) = (1, 1, 1, 1)
	_TexUV5 ("Texture using UV5", 2D) = "black" {}
	_Color6 ( "Color 6", Color) = (1, 1, 1, 1)
	_TexUV6 ("Texture using UV6", 2D) = "black" {}
	_Color7 ( "Color 7", Color) = (1, 1, 1, 1)
	_TexUV7 ("Texture using UV7", 2D) = "black" {}
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

        float4 _Color0;
        float4 _Color1;
        float4 _Color2;
        float4 _Color3;
        float4 _Color4;
        float4 _Color5;
        float4 _Color6;
        float4 _Color7;
		sampler2D _TexUV0;
		sampler2D _TexUV1;
		sampler2D _TexUV2;
		sampler2D _TexUV3;
		sampler2D _TexUV4;
		sampler2D _TexUV5;
		sampler2D _TexUV6;
		sampler2D _TexUV7;

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
		    float4 uv0 : TEXCOORD0;
		    float4 uv1 : TEXCOORD1;
		    float4 uv2 : TEXCOORD2;
		    float4 uv3 : TEXCOORD3;
		    float4 uv4 : TEXCOORD4;
		    float4 uv5 : TEXCOORD5;
		    float4 uv6 : TEXCOORD6;
		    float4 uv7 : TEXCOORD7;
        };

//----------------------------------------------------------------------------------------------------------------------
        PS_IN EightUV_VS(VS_IN v) {
            PS_IN o;
            o.pos = UnityObjectToClipPos(v.vertex);
        	o.uv0 = v.texcoord0;
        	o.uv1 = v.texcoord1;
        	o.uv2 = v.texcoord2;
        	o.uv3 = v.texcoord3;
        	o.uv4 = v.texcoord4;
        	o.uv5 = v.texcoord5;
        	o.uv6 = v.texcoord6;
        	o.uv7 = v.texcoord7;
            return o;
        }


        float4 EightUV_PS(PS_IN input) : COLOR {

        	const float4 tex0 = tex2D(_TexUV0, input.uv0).rgba;
        	const float4 tex1 = tex2D(_TexUV1, input.uv1).rgba;
        	const float4 tex2 = tex2D(_TexUV2, input.uv2).rgba;
        	const float4 tex3 = tex2D(_TexUV3, input.uv3).rgba;
        	const float4 tex4 = tex2D(_TexUV4, input.uv4).rgba;
        	const float4 tex5 = tex2D(_TexUV5, input.uv5).rgba;
        	const float4 tex6 = tex2D(_TexUV6, input.uv6).rgba;
        	const float4 tex7 = tex2D(_TexUV7, input.uv7).rgba;

       	
            return
        	      (tex0 * _Color0)
        	    + (tex1 * _Color1)        		
        		+ (tex2 * _Color2)
        		+ (tex3 * _Color3)
        		+ (tex4 * _Color4)
        		+ (tex5 * _Color5)
        		+ (tex6 * _Color6)
        		+ (tex7 * _Color7)
        	;
        }

		ENDCG
	}		

}
FallBack "Diffuse"

} //end Shader
