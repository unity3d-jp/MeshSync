using System;
using System.Collections.Generic;
using System.Linq;
using Unity.Mathematics;
using UnityEditor;
using UnityEngine;
using UnityEngine.Experimental.Rendering;

namespace Unity.MeshSync {
    internal class ShaderHelper {
        private const string _SmoothnessTextureChannel = "_SmoothnessTextureChannel";

        static Dictionary<string, ShaderHelper> loadedShaders = new();

        private static ShaderHelper LoadShader(string name) {
            string file = "meshsync_channel_mapping";

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

            // If there is no such texture, create one with a fallback value:

            float value = fallbackPropertyValue;

            for (int i = 0; i < materialProperties.Count; i++) {
                if (materialProperties[i].name == fallbackPropertyName) {
                    value = materialProperties[i].floatValue;
                    break;
                }
            }

            const int dim = 8;

            var pixels = Enumerable.Repeat(new Color(value, value, value, value), dim * dim).ToArray();

            texture = new Texture2D(dim, dim, UnityEngine.TextureFormat.RGBA32, true, true);

            texture.SetPixels(pixels);

            return false;
        }

        public static void BakeSmoothness(Material destMat,
            List<TextureHolder> textureHolders,
            List<MaterialPropertyData> materialProperties) {
            var smoothnessChannel = destMat.GetInt(_SmoothnessTextureChannel);

            Texture2D rgbTexture = null;
            string channelName = null;

            bool texturesExist = false;

            if (smoothnessChannel == 0) {
                // Bake to metallic alpha
                channelName = BaseMeshSync._MetallicGlossMap;
                texturesExist |= FindTexture(BaseMeshSync._MetallicGlossMap, textureHolders, materialProperties, BaseMeshSync._Metallic, 0, out rgbTexture);
            }
            else if (smoothnessChannel == 1) {
                // Bake to albedo alpha
                channelName = BaseMeshSync._BaseMap;
                texturesExist |= FindTexture(BaseMeshSync._MainTex, textureHolders, materialProperties, BaseMeshSync._Color, 0, out rgbTexture);
            }
            else {
                Debug.LogError($"Unknown smoothness channel, cannot bake to option: {smoothnessChannel} set in the smoothness texture channel.");
                return;
            }

            // Bake smoothness to 1 if there is no map and use the slider to scale it:
            texturesExist |= FindTexture(BaseMeshSync._GlossMap, textureHolders, materialProperties, string.Empty, 1, out var glossTexture);

            // If there are no textures, don't bake anything, slider values can control everything:
            if (!texturesExist ||
                glossTexture == null ||
                rgbTexture == null) {
                destMat.SetTexture(channelName, null);
                return;
            }

            var shader = LoadShader("smoothness_into_alpha");

            shader.SetTexture("Smoothness", glossTexture);
            shader.SetTexture("RGB", rgbTexture);

            var texture = shader.RenderToTexture();

            if (channelName == BaseMeshSync._MetallicGlossMap) {
                destMat.EnableKeyword(BaseMeshSync._METALLICGLOSSMAP);
            }
            else if (smoothnessChannel == 0) {
                destMat.DisableKeyword(BaseMeshSync._METALLICGLOSSMAP);
            }

            destMat.SetTexture(channelName, texture);

            //destMat.SetTexture(channelName, null);
        }

        //        private static Texture TextureFromRenderTexture(RenderTexture renderTarget, string path) {
        //            // TODO:
        //            // Saving the render texture out to the assetlibrary is really slow, use the render texture and save it later instead:
        //#if HMMMMMMMMMMMMM
        //            var texture = new Texture2D(renderTarget.width, renderTarget.height, UnityEngine.TextureFormat.RGBA32, true);
        //            RenderTexture.active = renderTarget;
        //            texture.ReadPixels(new Rect(0, 0, renderTarget.width, renderTarget.height), 0, 0);
        //            texture.Apply();


        //            TextureData.WriteToFile(path, texture.EncodeToPNG());

        //            if (string.IsNullOrEmpty(AssetDatabase.AssetPathToGUID(path))) { 
        //                AssetDatabase.ImportAsset(path);
        //            }
        //            var savedTexture = AssetDatabase.LoadAssetAtPath<Texture2D>(path);

        //            return savedTexture;
        //#endif

        //            //AssetDatabase.upda

        //            return renderTarget;
        //        }

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

        public Texture RenderToTexture() {
            var renderTarget = new RenderTexture(maxTextureSize.x, maxTextureSize.y, 32) {
                enableRandomWrite = true
            };
            renderTarget.Create();
            SetTexture("Output", renderTarget);

            var groupsX = (int)Math.Max(1, Math.Ceiling(maxTextureSize.x / (float)groupSizeX));
            var groupsY = (int)Math.Max(1, Math.Ceiling(maxTextureSize.y / (float)groupSizeY));

            shader.Dispatch(kernelIndex, groupsX, groupsY, 1);

            //var texture = TextureFromRenderTexture(renderTarget, outputTexturePath);

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
