Shader "MeshSync/Points Standard" {
    Properties {
        _Color("Color", Color) = (1,1,1,1)
        _MainTex ("Albedo (RGB)", 2D) = "white" {}
        _Glossiness ("Smoothness", Range(0,1)) = 0.5
        _Metallic ("Metallic", Range(0,1)) = 0.0
    }

    SubShader {
        Tags { "RenderType"="Opaque" }

        CGPROGRAM
        #pragma surface surf Standard fullforwardshadows addshadow
        #pragma target 4.5
        #pragma multi_compile_instancing
        #pragma instancing_options assumeuniformscaling procedural:setup
        #include "PointRenderer.cginc"

        fixed4 _Color;
        sampler2D _MainTex;

        struct Input {
            float2 uv_MainTex;
        };

        half _Glossiness;
        half _Metallic;

#ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
        void setup()
        {
            unity_ObjectToWorld = GetPointMatrix(unity_InstanceID);
            unity_WorldToObject = unity_ObjectToWorld;
            unity_WorldToObject._14_24_34 *= -1;
            unity_WorldToObject._11_22_33 = 1.0f / unity_WorldToObject._11_22_33;
        }
#endif

        void surf(Input IN, inout SurfaceOutputStandard o)
        {
            float4 color = _Color;
#ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
            color *= GetPointColor(unity_InstanceID);
#endif

            fixed4 c = tex2D(_MainTex, IN.uv_MainTex) * color;
            o.Albedo = c.rgb;
            o.Metallic = _Metallic;
            o.Smoothness = _Glossiness;
            o.Alpha = c.a;
        }
        ENDCG
    }
}
