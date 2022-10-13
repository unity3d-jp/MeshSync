using System.Collections.Generic;
using System.Linq;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync {
    /// <summary>
    /// Helper class to bake smoothness and metallic into channels required for the standard shaders.
    /// </summary>
    internal static class MapsBaker {
        private const string _SmoothnessTextureChannel = "_SmoothnessTextureChannel";

        static Dictionary<string, ComputeShaderHelper> loadedShaders = new Dictionary<string, ComputeShaderHelper>();

        private const string SHADER_CONST_METALLIC = "Metallic";
        private const string SHADER_CONST_SMOOTHNESS = "Smoothness";
        private const string SHADER_CONST_RGB = "RGB";

        private const string SHADER_FILE = "meshsync_channel_mapping";
        private const string SHADER_NAME_SMOOTHNESS_INTO_ALPHA = "smoothness_into_alpha";
        private const string SHADER_NAME_HDRP_MASK = "hdrp_mask";


        private static ComputeShaderHelper LoadShader(string name) {
            string file = SHADER_FILE;

            if (!loadedShaders.TryGetValue(file, out var shaderHelper)) {
#if UNITY_EDITOR
                var shaderFiles = AssetDatabase.FindAssets(file);
                if (shaderFiles.Length > 0) {
                    var shaderFile = shaderFiles[0];
                    var shader = AssetDatabase.LoadAssetAtPath<ComputeShader>(AssetDatabase.GUIDToAssetPath(shaderFile));

                    shaderHelper = new ComputeShaderHelper(shader, name);

                    loadedShaders[name] = shaderHelper;
                }
#else
// TODO: Load shader at runtime here
#endif
            }

            return shaderHelper;
        }

        static MaterialPropertyData? FindMaterialProperty(List<MaterialPropertyData> materialProperties, string name) {
            for (int i = 0; i < materialProperties.Count; i++) {
                if (materialProperties[i].name == name) {
                    return materialProperties[i];
                }
            }

            return null;
        }

        private static bool FindTexture(string name,
            List<TextureHolder> textureHolders,
            List<MaterialPropertyData> materialProperties,
            string fallbackPropertyName,
            float fallbackPropertyValue,
            out Texture2DDisposable disposableTexture) {

            var prop = FindMaterialProperty(materialProperties, name);

            if (prop.HasValue) {
                MaterialPropertyData.TextureRecord rec = prop.Value.textureValue;
                disposableTexture = new Texture2DDisposable(BaseMeshSync.FindTexture(rec.id, textureHolders), false);

                if (disposableTexture.Texture != null) {
                    return true;
                }
            }

            // If there is no such texture, create one with the given fallback value:

            float value = fallbackPropertyValue;

            for (int i = 0; i < materialProperties.Count; i++) {
                if (materialProperties[i].name == fallbackPropertyName) {
                    value = materialProperties[i].floatValue;
                    break;
                }
            }

            const int dim = 8;

            var pixels = Enumerable.Repeat(new Color(value, value, value, value), dim * dim).ToArray();

            disposableTexture = new Texture2DDisposable(new Texture2D(dim, dim, UnityEngine.TextureFormat.RFloat, false, true));

            disposableTexture.Texture.SetPixels(pixels);
            disposableTexture.Texture.Apply();
            
            return false;
        }

#if AT_USE_HDRP
        private static void BakeMaskMap(Material destMat,
            List<TextureHolder> textureHolders,
            List<MaterialPropertyData> materialProperties) {
            if (!destMat.HasTexture(MeshSyncConstants._MaskMap)) {
                return;
            }

            // Mask map:
            // R: metal
            // G: ao
            // B: detail mask
            // A: smoothness

            bool texturesExist = false;

            texturesExist |=
                FindTexture(MeshSyncConstants._MetallicGlossMap, textureHolders, materialProperties,
                    MeshSyncConstants._Metallic, 0, out var metalTexture);

            texturesExist |=
                FindTexture(MeshSyncConstants._GlossMap, textureHolders, materialProperties,
                    MeshSyncConstants._Glossiness, 1, out var glossTexture);

            // If there are no textures, don't bake anything, slider values can control everything:
            if (!texturesExist ||
                metalTexture.Texture == null ||
                glossTexture.Texture == null) {
                destMat.SetTexture(MeshSyncConstants._MaskMap, null);
                destMat.DisableKeyword(MeshSyncConstants._MASKMAP);
                return;
            }

            var shader = LoadShader(SHADER_NAME_HDRP_MASK);
            if (shader == null) {
                return;
            }

            shader.SetTexture(SHADER_CONST_METALLIC, metalTexture.Texture);
            shader.SetTexture(SHADER_CONST_SMOOTHNESS, glossTexture.Texture);

            var texture = shader.RenderToTexture(destMat.GetTexture(MeshSyncConstants._MaskMap));

            destMat.EnableKeyword(MeshSyncConstants._MASKMAP);

            destMat.SetTexture(MeshSyncConstants._MaskMap, texture);
        }
#else
        private static void BakeSmoothness(Material destMat,
            List<TextureHolder> textureHolders,
            List<MaterialPropertyData> materialProperties) {
         
            if (!destMat.HasProperty(_SmoothnessTextureChannel)) {
                return;
            }

            var smoothnessChannel = destMat.GetInt(_SmoothnessTextureChannel);

            Texture2DDisposable rgbTexture = null;
            string channelName = null;

            bool texturesExist = false;

            if (smoothnessChannel == 0) {
                // Bake to metallic alpha
                channelName = MeshSyncConstants._MetallicGlossMap;
                texturesExist |=
                    FindTexture(MeshSyncConstants._MetallicGlossMap, textureHolders, materialProperties,
                        MeshSyncConstants._Metallic, 0, out rgbTexture);
            }
            else if (smoothnessChannel == 1) {
                // Bake to albedo alpha
                channelName = MeshSyncConstants._BaseMap;
                texturesExist |=
                    FindTexture(MeshSyncConstants._MainTex, textureHolders, materialProperties, MeshSyncConstants._Color, 0,
                        out rgbTexture);
            }
            else {
                Debug.LogError(
                    $"Unknown smoothness channel, cannot bake to option: {smoothnessChannel} set in the smoothness texture channel.");
                return;
            }

            // Bake smoothness to 1 if there is no map and use the slider to scale it:
            texturesExist |=
                FindTexture(MeshSyncConstants._GlossMap, textureHolders, materialProperties, string.Empty, 1,
                    out var glossTexture);

            // If there are no textures, don't bake anything, slider values can control everything:
            if (!texturesExist ||
                glossTexture.Texture == null ||
                rgbTexture.Texture == null) {
                destMat.SetTexture(channelName, null);

                if (channelName == MeshSyncConstants._MetallicGlossMap) {
                    destMat.DisableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
                    destMat.DisableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);
                }

                return;
            }

            var shader = LoadShader(SHADER_NAME_SMOOTHNESS_INTO_ALPHA);
            if (shader == null) {
                return;
            }

            shader.SetTexture(SHADER_CONST_SMOOTHNESS, glossTexture.Texture);
            shader.SetTexture(SHADER_CONST_RGB, rgbTexture.Texture);

            var texture = shader.RenderToTexture(destMat.GetTexture(channelName));

            if (channelName == MeshSyncConstants._MetallicGlossMap) {
                destMat.EnableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
                destMat.EnableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);
            }
            else if (smoothnessChannel == 0) {
                destMat.DisableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
                destMat.DisableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);
            }

            destMat.SetTexture(channelName, texture);

            glossTexture.Dispose();
            rgbTexture.Dispose();
        }
#endif

        public static void BakeMaps(Material destMat,
            List<TextureHolder> textureHolders,
            List<MaterialPropertyData> materialProperties) {
#if AT_USE_HDRP
            BakeMaskMap(destMat, textureHolders, materialProperties);
#else
            BakeSmoothness(destMat, textureHolders, materialProperties);
#endif
        }
    }
}
