using UnityEngine;

namespace Unity.MeshSync {
internal static class MeshSyncConstants {
    internal const int    DEFAULT_SERVER_PORT             = 8080;
    internal const string DEFAULT_SCENE_CACHE_OUTPUT_PATH = "Assets/SceneCacheResources";

    internal const string PACKAGE_NAME = "com.unity.meshsync";

    internal const string DEFAULT_ASSETS_PATH = "Assets/MeshSyncAssets";

    #region Material properties

    internal static readonly int _Color                    = Shader.PropertyToID("_Color");
    internal static readonly int _BaseColor                = Shader.PropertyToID("_BaseColor");
    internal static readonly int _MainTex                  = Shader.PropertyToID("_MainTex");
    internal static readonly int _BaseColorMap             = Shader.PropertyToID("_BaseColorMap");
    internal static readonly int _BaseMap                  = Shader.PropertyToID("_BaseMap");
    internal static readonly int _GlossMap                 = Shader.PropertyToID("_GlossMap");
    internal static readonly int _EmissionColor            = Shader.PropertyToID("_EmissionColor");
    internal static readonly int _EmissionMap              = Shader.PropertyToID("_EmissionMap");
    internal static readonly int _EmissiveColorMap         = Shader.PropertyToID("_EmissiveColorMap");
    internal static readonly int _EmissiveIntensity        = Shader.PropertyToID("_EmissiveIntensity");
    internal static readonly int _EmissiveIntensityUnit    = Shader.PropertyToID("_EmissiveIntensityUnit");
    internal static readonly int _EmissiveColorLDR         = Shader.PropertyToID("_EmissiveColorLDR");
    internal static readonly int _EmissiveColor            = Shader.PropertyToID("_EmissiveColor");
    internal static readonly int _EmissionStrength         = Shader.PropertyToID("_EmissionStrength");
    internal static readonly int _UseEmissiveIntensity     = Shader.PropertyToID("_UseEmissiveIntensity");
    internal static readonly int _Metallic                 = Shader.PropertyToID("_Metallic");
    internal static readonly int _Glossiness               = Shader.PropertyToID("_Glossiness");
    internal static readonly int _Smoothness               = Shader.PropertyToID("_Smoothness");
    internal static readonly int _GlossMapScale            = Shader.PropertyToID("_GlossMapScale");
    internal static readonly int _MetallicGlossMap         = Shader.PropertyToID("_MetallicGlossMap");
    internal static readonly int _BumpMap                  = Shader.PropertyToID("_BumpMap");
    internal static readonly int _BumpScale                = Shader.PropertyToID("_BumpScale");
    internal static readonly int _NormalScale              = Shader.PropertyToID("_NormalScale");
    internal static readonly int _NormalMap                = Shader.PropertyToID("_NormalMap");
    internal static readonly int _ParallaxMap              = Shader.PropertyToID("_ParallaxMap");
    internal static readonly int _Parallax                 = Shader.PropertyToID("_Parallax");
    internal static readonly int _HeightMap                = Shader.PropertyToID("_HeightMap");
    internal static readonly int _AlphaClip                = Shader.PropertyToID("_AlphaClip");
    internal static readonly int _AlphaCutoffEnable        = Shader.PropertyToID("_AlphaCutoffEnable");
    internal static readonly int _MaskMap                  = Shader.PropertyToID("_MaskMap");
    internal static readonly int _Blend                    = Shader.PropertyToID("_Blend");
    internal static readonly int _SrcBlend                 = Shader.PropertyToID("_SrcBlend");
    internal static readonly int _DstBlend                 = Shader.PropertyToID("_DstBlend");
    internal static readonly int _AlphaSrcBlend            = Shader.PropertyToID("_AlphaSrcBlend");
    internal static readonly int _AlphaDstBlend            = Shader.PropertyToID("_AlphaDstBlend");
    internal static readonly int _ZWrite                   = Shader.PropertyToID("_ZWrite");
    internal static readonly int _SurfaceType              = Shader.PropertyToID("_SurfaceType");
    internal static readonly int _ZTestDepthEqualForOpaque = Shader.PropertyToID("_ZTestDepthEqualForOpaque");
    internal static readonly int _Mode                     = Shader.PropertyToID("_Mode");
    internal static readonly int _Surface                  = Shader.PropertyToID("_Surface");
    internal static readonly int _DisplacementMode         = Shader.PropertyToID("_DisplacementMode");
    internal static readonly int _HeightMin                = Shader.PropertyToID("_HeightMin");
    internal static readonly int _HeightMax                = Shader.PropertyToID("_HeightMax");
    internal static readonly int _HeightAmplitude          = Shader.PropertyToID("_HeightAmplitude");
    internal static readonly int _ZTestGBuffer             = Shader.PropertyToID("_ZTestGBuffer");
    internal static readonly int _CoatMaskMap              = Shader.PropertyToID("_CoatMaskMap");
    internal static readonly int _CoatMask                 = Shader.PropertyToID("_CoatMask");
    
    #endregion

    #region Keywords

    internal const string _MASKMAP                     = "_MASKMAP";
    internal const string _ALPHATEST_ON                = "_ALPHATEST_ON";
    internal const string _ALPHABLEND_ON               = "_ALPHABLEND_ON";
    internal const string _ALPHAPREMULTIPLY_ON         = "_ALPHAPREMULTIPLY_ON";
    internal const string _HEIGHTMAP                   = "_HEIGHTMAP";
    internal const string _PARALLAXMAP                 = "_PARALLAXMAP";
    internal const string _NORMALMAP                   = "_NORMALMAP";
    internal const string _METALLICGLOSSMAP            = "_METALLICGLOSSMAP";
    internal const string _METALLICSPECGLOSSMAP        = "_METALLICSPECGLOSSMAP";
    internal const string _EMISSION                    = "_EMISSION";
    internal const string _ENABLE_FOG_ON_TRANSPARENT   = "_ENABLE_FOG_ON_TRANSPARENT";
    internal const string _SURFACE_TYPE_TRANSPARENT    = "_SURFACE_TYPE_TRANSPARENT";
    internal const string _PIXEL_DISPLACEMENT          = "_PIXEL_DISPLACEMENT";
    internal const string _VERTEX_DISPLACEMENT         = "_VERTEX_DISPLACEMENT";
    internal const string _MATERIAL_FEATURE_CLEAR_COAT = "_MATERIAL_FEATURE_CLEAR_COAT";

    // Used to mark materials that have a specific shader setup based on a shader from the DCC tool.  
    internal const string MESHSYNC_OVERRIDE = "MESHSYNC_OVERRIDE";

    #endregion

    #region Override tags

    internal const string RenderType        = "RenderType";
    internal const string Transparent       = "Transparent";
    internal const string TransparentCutout = "TransparentCutout";

    #endregion
}
} //end namespace