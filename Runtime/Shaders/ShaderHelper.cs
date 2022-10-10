using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using UnityEditor;
using UnityEngine;
using UnityEngine.Experimental.Rendering;

namespace Unity.MeshSync {
    internal class ShaderHelper {
        private const string _SmoothnessTextureChannel = "_SmoothnessTextureChannel";

        static Dictionary<string, ShaderHelper> loadedShaders = new();
        
        private const string SHADER_CONST_OUTPUT     = "Output";
        private const string SHADER_CONST_METALLIC   = "Metallic";
        private const string SHADER_CONST_SMOOTHNESS = "Smoothness";
        private const string SHADER_CONST_RGB        = "RGB";

        private const string SHADER_FILE = "meshsync_channel_mapping";
        private const string SHADER_NAME_SMOOTHNESS_INTO_ALPHA = "smoothness_into_alpha";
        private const string SHADER_NAME_HDRP_MASK             = "hdrp_mask";
        

        private static ShaderHelper LoadShader(string name) {
            string file = SHADER_FILE;

            if (!loadedShaders.TryGetValue(file, out var shaderHelper)) {
                var shaderFiles = AssetDatabase.FindAssets(file);
                if (shaderFiles.Length > 0) {
                    var shaderFile = shaderFiles[0];
                    var shader = AssetDatabase.LoadAssetAtPath<ComputeShader>(AssetDatabase.GUIDToAssetPath(shaderFile));

                    shaderHelper = new ShaderHelper(shader, name);

                    loadedShaders[name] = shaderHelper;
                }
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
            out Texture2D texture) {
            var prop = FindMaterialProperty(materialProperties, name);

            if (prop.HasValue) {
                MaterialPropertyData.TextureRecord rec = prop.Value.textureValue;
                texture = BaseMeshSync.FindTexture(rec.id, textureHolders);

                if (texture != null) {
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

            texture = new Texture2D(dim, dim, UnityEngine.TextureFormat.RFloat, false, true);

            texture.SetPixels(pixels);
            texture.Apply();

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
                metalTexture == null ||
                glossTexture == null) {
                destMat.SetTexture(MeshSyncConstants._MaskMap, null);
                destMat.DisableKeyword(MeshSyncConstants._MASKMAP);
                return;
            }

            var shader = LoadShader(SHADER_NAME_HDRP_MASK);

            shader.SetTexture(SHADER_CONST_METALLIC, metalTexture);
            shader.SetTexture(SHADER_CONST_SMOOTHNESS, glossTexture);

            var texture = shader.RenderToTexture(destMat.GetTexture(MeshSyncConstants._MaskMap));

            destMat.EnableKeyword(MeshSyncConstants._MASKMAP);

            destMat.SetTexture(MeshSyncConstants._MaskMap, texture);
        }
#else
        private static void BakeSmoothness(Material destMat,
            List<TextureHolder> textureHolders,
            List<MaterialPropertyData> materialProperties) {
            if (!destMat.HasInt(_SmoothnessTextureChannel)) {
                return;
            }

            var smoothnessChannel = destMat.GetInt(_SmoothnessTextureChannel);

            Texture2D rgbTexture  = null;
            string    channelName = null;

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
                glossTexture == null ||
                rgbTexture == null) {
                destMat.SetTexture(channelName, null);

                if (channelName == MeshSyncConstants._MetallicGlossMap) {
                    destMat.DisableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
                    destMat.DisableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);
                }

                return;
            }

            var shader = LoadShader(SHADER_NAME_SMOOTHNESS_INTO_ALPHA);

            shader.SetTexture(SHADER_CONST_SMOOTHNESS, glossTexture);
            shader.SetTexture(SHADER_CONST_RGB, rgbTexture);

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
        
        #region Internals

        private readonly ComputeShader shader;
        int kernelIndex;

        readonly uint groupSizeX;
        readonly uint groupSizeY;

        Vector2Int maxTextureSize = new Vector2Int(1, 1);

        public ShaderHelper(ComputeShader shader, string kernelName) {
            this.shader = shader;

            kernelIndex = shader.FindKernel(kernelName);
            shader.GetKernelThreadGroupSizes(kernelIndex, out groupSizeX, out groupSizeY, out var gsZ);
        }

        public Texture RenderToTexture(Texture existingTexture) {
            var renderTarget = existingTexture as RenderTexture;
            
            // If there is an existing renderTexture, reuse it:
            if (renderTarget == null ||
                renderTarget.width != maxTextureSize.x || 
                renderTarget.height != maxTextureSize.y) {
                renderTarget = new RenderTexture(maxTextureSize.x, maxTextureSize.y, 32) {
                    enableRandomWrite = true
                };
                
                renderTarget.Create();
            }

            SetTexture(SHADER_CONST_OUTPUT, renderTarget);

            var groupsX = (int)Math.Max(1, Math.Ceiling(maxTextureSize.x / (float)groupSizeX));
            var groupsY = (int)Math.Max(1, Math.Ceiling(maxTextureSize.y / (float)groupSizeY));

            shader.Dispatch(kernelIndex, groupsX, groupsY, 1);
            
            return renderTarget;
        }

        public void SetTexture(string name, Texture texture) {
            shader.SetTexture(kernelIndex, name, texture);

            maxTextureSize.x = Math.Max(maxTextureSize.x, texture.width);
            maxTextureSize.y = Math.Max(maxTextureSize.y, texture.height);
        }

        #endregion
    }
}
