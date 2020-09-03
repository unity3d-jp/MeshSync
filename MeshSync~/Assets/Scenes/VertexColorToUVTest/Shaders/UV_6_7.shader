Shader "VertexColorToUVTest/UV_6_7" {
	SubShader{
		Tags { "RenderType" = "Opaque" }
		LOD 200

		CGPROGRAM
		#pragma surface surf Lambert vertex:vert
		#pragma target 3.0

		struct appdata {
			float4 vertex : POSITION;
			float4 tangent : TANGENT;
			float3 normal : NORMAL;
			float4 texcoord : TEXCOORD0;
			float4 texcoord1 : TEXCOORD1;
			float4 texcoord2 : TEXCOORD2;
			float4 texcoord3 : TEXCOORD3;

			float4 texcoord4 : TEXCOORD4;
			float4 texcoord5 : TEXCOORD5;
			float4 texcoord6 : TEXCOORD6;
			float4 texcoord7 : TEXCOORD7;

			fixed4 color : COLOR;
		};

		struct Input {
			float4 vertColor;
		};

		void vert(inout appdata v, out Input o) {
			UNITY_INITIALIZE_OUTPUT(Input, o);
			//o.vertColor = v.color;
			o.vertColor = float4(v.texcoord6.xy, v.texcoord7.xy);
		}

		void surf(Input IN, inout SurfaceOutput o) {
			o.Albedo = IN.vertColor.rgb;
		}
		ENDCG
	}
		FallBack "Diffuse"
}