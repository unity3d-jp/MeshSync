Shader "Hidden/HumbleNormalEditor/Visualizer" {

CGINCLUDE
#include "UnityCG.cginc"

float _VertexSize;
float _NormalSize;
float _TangentSize;
float _BinormalSize;

float4 _VertexColor;
float4 _VertexColor2;
float4 _VertexColor3;
float4 _NormalColor;
float4 _TangentColor;
float4 _BinormalColor;
float3 _RayPos;
float3 _RayRadPow;
int _OnlySelected = 0;

float4x4 _Transform;
StructuredBuffer<float3> _BaseNormals;
StructuredBuffer<float4> _BaseTangents;
StructuredBuffer<float3> _Points;
StructuredBuffer<float3> _Normals;
StructuredBuffer<float4> _Tangents;
StructuredBuffer<float> _Selection;

struct appdata
{
    float4 vertex : POSITION;
    float4 normal : NORMAL;
    float4 uv : TEXCOORD0;
    float4 color : COLOR;
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct v2f
{
    float4 vertex : SV_POSITION;
    float4 color : TEXCOORD0;
};

v2f vert_vertices(appdata v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;

    float s = _Selection[v.instanceID];
    float4 vertex = v.vertex;
    vertex.xyz *= _VertexSize;
    vertex.xyz *= abs(UnityObjectToViewPos(pos).z);
    vertex.xyz += pos;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    v2f o;
    o.vertex = vertex;

    float d = clamp(pow(clamp(1.0f - length(pos - _RayPos) / _RayRadPow.x, 0, 1), _RayRadPow.y), 0, 1);
    o.color = lerp(_VertexColor, _VertexColor2, s);
    o.color.rgb += _VertexColor3.rgb * d;
    return o;
}

v2f vert_normals(appdata v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;
    float3 dir = normalize((mul(_Transform, float4(_Normals[v.instanceID], 0.0))).xyz);

    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += pos + dir * v.uv.x * _NormalSize * s;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _NormalColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

v2f vert_tangents(appdata v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;
    float3 dir = normalize((mul(_Transform, float4(_Tangents[v.instanceID].xyz, 0.0))).xyz);

    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += pos + dir * v.uv.x * _TangentSize * s;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _TangentColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}

v2f vert_binormals(appdata v)
{
    float3 pos = (mul(_Transform, float4(_Points[v.instanceID], 1.0))).xyz;
    float3 binormal = normalize(cross(_Normals[v.instanceID], _Tangents[v.instanceID].xyz) * _Tangents[v.instanceID].w);
    float3 dir = normalize((mul(_Transform, float4(binormal, 0.0))).xyz);

    float s = _OnlySelected ? _Selection[v.instanceID] : 1.0f;
    float4 vertex = v.vertex;
    vertex.xyz += pos + dir * v.uv.x * _BinormalSize * s;
    vertex = mul(UNITY_MATRIX_VP, vertex);

    v2f o;
    o.vertex = vertex;
    o.color = _BinormalColor;
    o.color.a = 1.0 - v.uv.x;
    return o;
}


v2f vert_local_space_normals(appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.color.rgb = v.normal.xyz * 0.5 + 0.5;
    o.color.a = 1.0;
    return o;
}

float3 ToBaseTangentSpace(uint vid, float3 n)
{
    float3 base_normal = _BaseNormals[vid];
    float4 base_tangent = _BaseTangents[vid];
    float3 base_binormal = normalize(cross(base_normal, base_tangent.xyz) * base_tangent.w);
    float3x3 tbn = float3x3(base_tangent.xyz, base_binormal, base_normal);
    return normalize(mul(n, transpose(tbn)));
}

v2f vert_tangent_space_normals(appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.color.rgb = ToBaseTangentSpace(v.vertexID, v.normal.xyz) * 0.5 + 0.5;
    o.color.a = 1.0;
    return o;
}

v2f vert_color(appdata v)
{
    v2f o;
    o.vertex = UnityObjectToClipPos(v.vertex);
    o.color = v.color;
    return o;
}

v2f vert_lasso(appdata v)
{
    v2f o;
    o.vertex = float4(v.vertex.xy, 0.0, 1.0);
    o.vertex.y *= -1;
    o.color = float4(1.0, 0.0, 0.0, 1.0);
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
        ZWrite Off
        Blend SrcAlpha OneMinusSrcAlpha

        // pass 0: visualize vertices
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_vertices
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 1: visualize normals
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 2: visualize tangents
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_tangents
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 3: visualize binormals
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_binormals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }
            
        // pass 4: local space normals overlay
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_local_space_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 5: tangent space normals overlay
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_tangent_space_normals
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 6: vertex color overlay
        Pass
        {
            ZTest LEqual

            CGPROGRAM
            #pragma vertex vert_color
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }

        // pass 7: lasso
        Pass
        {
            ZTest Always

            CGPROGRAM
            #pragma vertex vert_lasso
            #pragma fragment frag
            #pragma target 4.5
            ENDCG
        }
    }
}
