using UnityEngine;

namespace Unity.MeshSync {
    internal static class MeshSyncConstants {
        internal const int DEFAULT_SERVER_PORT = 8080;
        internal const string DEFAULT_SCENE_CACHE_OUTPUT_PATH = "Assets/SceneCacheResources";

        internal const string PACKAGE_NAME = "com.unity.meshsync";

        internal const string DEFAULT_ASSETS_PATH = "Assets/MeshSyncAssets";

        #region Material properties

        internal static readonly int _Color = Shader.PropertyToID("_Color");
        internal static readonly int _BaseColor = Shader.PropertyToID("_BaseColor");
        internal static readonly int _MainTex = Shader.PropertyToID("_MainTex");
        internal static readonly int _BaseColorMap = Shader.PropertyToID("_BaseColorMap");
        internal static readonly int _BaseMap = Shader.PropertyToID("_BaseMap");
        internal static readonly int _GlossMap = Shader.PropertyToID("_GlossMap");
        internal static readonly int _EmissionColor = Shader.PropertyToID("_EmissionColor");
        internal static readonly int _EmissionMap = Shader.PropertyToID("_EmissionMap");
        internal static readonly int _EmissiveColorMap = Shader.PropertyToID("_EmissiveColorMap");
        internal static readonly int _EmissiveIntensity = Shader.PropertyToID("_EmissiveIntensity");
        internal static readonly int _EmissiveIntensityUnit = Shader.PropertyToID("_EmissiveIntensityUnit");
        internal static readonly int _EmissiveColorLDR = Shader.PropertyToID("_EmissiveColorLDR");
        internal static readonly int _EmissiveColor = Shader.PropertyToID("_EmissiveColor");
        internal static readonly int _EmissionStrength = Shader.PropertyToID("_EmissionStrength");
        internal static readonly int _UseEmissiveIntensity = Shader.PropertyToID("_UseEmissiveIntensity");
        internal static readonly int _Metallic = Shader.PropertyToID("_Metallic");
        internal static readonly int _Glossiness = Shader.PropertyToID("_Glossiness");
        internal static readonly int _Smoothness = Shader.PropertyToID("_Smoothness");
        internal static readonly int _MetallicGlossMap = Shader.PropertyToID("_MetallicGlossMap");
        internal static readonly int _BumpMap = Shader.PropertyToID("_BumpMap");
        internal static readonly int _BumpScale = Shader.PropertyToID("_BumpScale");
        internal static readonly int _NormalScale = Shader.PropertyToID("_NormalScale");
        internal static readonly int _NormalMap = Shader.PropertyToID("_NormalMap");
        internal static readonly int _ParallaxMap = Shader.PropertyToID("_ParallaxMap");
        internal static readonly int _Parallax = Shader.PropertyToID("_Parallax");
        internal static readonly int _HeightMap = Shader.PropertyToID("_HeightMap");
        internal static readonly int _AlphaClip = Shader.PropertyToID("_AlphaClip");
        internal static readonly int _AlphaCutoffEnable = Shader.PropertyToID("_AlphaCutoffEnable");
        internal static readonly int _MaskMap = Shader.PropertyToID("_MaskMap");

        #endregion

        #region Keywords

        internal const string _MASKMAP              = "_MASKMAP";
        internal const string _ALPHATEST_ON         = "_ALPHATEST_ON";
        internal const string _HEIGHTMAP            = "_HEIGHTMAP";
        internal const string _PARALLAXMAP          = "_PARALLAXMAP";
        internal const string _NORMALMAP            = "_NORMALMAP";
        internal const string _METALLICGLOSSMAP     = "_METALLICGLOSSMAP";
        internal const string _METALLICSPECGLOSSMAP = "_METALLICSPECGLOSSMAP";
        internal const string _EMISSION             = "_EMISSION";

        #endregion
    }
} //end namespace