using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.MeshSync {
    partial class BaseMeshSync {
        static readonly string[] textureNames = {
            MeshSyncConstants._MetallicGlossMap,
            MeshSyncConstants._BaseMap,
            MeshSyncConstants._MaskMap
    };

        private List<Tuple<Material, string>> pendingMaterialUpdates = new();

        private string GetSavePath(Material mat, string textureName) {
            return Path.Combine(GetAssetsFolder(), $"{mat.name}_{textureName}_baked.png");
        }

        private void SaveMaterialRenderTexturesToAssetDatabase() {
            // Check if there are render textures that need to be saved to the asset database:
            foreach (MaterialHolder materialHolder in materialList) {
                foreach (string textureName in textureNames) {
                    var mat = materialHolder.material;
                    if (mat == null || !mat.HasTexture(textureName)) {
                        continue;
                    }

                    var tex = mat.GetTexture(textureName);

                    if (tex is RenderTexture renderTarget) {
                        var texture = new Texture2D(renderTarget.width, renderTarget.height,
                            UnityEngine.TextureFormat.RGBA32, true);
                        RenderTexture.active = renderTarget;
                        texture.ReadPixels(new Rect(0, 0, renderTarget.width, renderTarget.height), 0, 0);
                        texture.Apply();

                        var savePath = GetSavePath(mat, textureName);
                        TextureData.WriteToFile(savePath, texture.EncodeToPNG());

                        // Setting the texture can fail if the asset database has not imported it yet.
                        // If that happens, save it for later and try again:
                        if (!SetSerializedTextureForMaterial(mat, textureName)) {
                            pendingMaterialUpdates.Add(new Tuple<Material, string>(mat, textureName));

                            // Set the texture on the material to null for now so the render texture
                            // is not saved to file again if this gets called again before it was
                            // set to the texture in the asset library:
                            mat.SetTexture(textureName, null);
                        }
                    }
                }
            }
        }

        private bool SetSerializedTextureForMaterial(Material mat, string textureName) {
            var savePath = GetSavePath(mat, textureName);

            // LoadAssetAtPath can throw an exception if it's called during a domain backup but we have no way to detect that:
            try {
                if (AssetDatabase.LoadAssetAtPath<Texture2D>(savePath) == null) {
                    EditorApplication.delayCall -= UpdatePendingMaterials;
                    EditorApplication.delayCall += UpdatePendingMaterials;

                    return false;
                }
            }
            catch (Exception ex) {
                return false;
            }

            // Ensure the texture is up to date:
            AssetDatabase.ImportAsset(savePath);

            var savedTexture = AssetDatabase.LoadAssetAtPath<Texture2D>(savePath);

            mat.SetTexture(textureName, savedTexture);

            if (textureName == MeshSyncConstants._MetallicGlossMap ||
                textureName == MeshSyncConstants._MaskMap) {
                mat.EnableKeyword(MeshSyncConstants._METALLICGLOSSMAP);
                mat.EnableKeyword(MeshSyncConstants._METALLICSPECGLOSSMAP);

                TextureImporter importer = (TextureImporter)AssetImporter.GetAtPath(savePath);
                if (importer != null) {
                    importer.sRGBTexture = false;
                    importer.alphaIsTransparency = false;
                    importer.ignorePngGamma = true;
                }
            }

            return true;
        }

        private void UpdatePendingMaterials() {
            EditorApplication.delayCall -= UpdatePendingMaterials;

            for (int i = pendingMaterialUpdates.Count - 1; i >= 0; i--) {
                Tuple<Material, string> pendingMaterial = pendingMaterialUpdates[i];
                if (SetSerializedTextureForMaterial(pendingMaterial.Item1, pendingMaterial.Item2)) {
                    pendingMaterialUpdates.RemoveAt(i);
                }
            }
        }

        private static void UpdateShader(ref Material mat, string shaderName) {
            Shader shader = null;
            if (!string.IsNullOrEmpty(shaderName)) {
                shader = Shader.Find(shaderName);
            }

            bool shaderExists = shader != null;

            if (shader == null) {
#if AT_USE_HDRP
                shader = Shader.Find("HDRP/Lit");
#elif AT_USE_URP
                shader = Shader.Find("Universal Render Pipeline/Lit");
#else
                shader = Shader.Find("Standard");
#endif
            }

            Assert.IsNotNull(shader);

            if (mat == null) {
                mat = new Material(shader);
            }
            else {
                mat.shader = shader;
            }

            bool usingOverride = false;

            // If the given shader name did not exist, try to set up the material to be close to the given shader name:
            if (!shaderExists) {
                if (shaderName.ToLower() == "glass") {
                    mat.EnableKeyword("MESHSYNC_OVERRIDE");
                    usingOverride = true;

                    mat.EnableKeyword("_SURFACE_TYPE_TRANSPARENT");
                    mat.SetOverrideTag("RenderType", "Transparent");
                    mat.SetFloat("_Surface", 1);
#if AT_USE_HDRP
                    mat.EnableKeyword("_ENABLE_FOG_ON_TRANSPARENT");

                    mat.renderQueue = 3000;

                    mat.SetFloat("_SurfaceType", 1);
                    mat.SetInt("_ZWrite", 0);
                    mat.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.One);
                    mat.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
                    mat.SetFloat("_AlphaSrcBlend", (int)UnityEngine.Rendering.BlendMode.One);
                    mat.SetFloat("_AlphaDstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
                    mat.SetFloat("_ZTestDepthEqualForOpaque", 4);
#elif AT_USE_URP
                mat.SetFloat("_Blend", 1);
                mat.DisableKeyword("_ALPHATEST_ON");
                mat.DisableKeyword("_ALPHABLEND_ON");
                mat.EnableKeyword("_ALPHAPREMULTIPLY_ON");
                mat.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.One);
                mat.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
#else
                    mat.SetFloat("_Mode", 3);
                    mat.DisableKeyword("_ALPHATEST_ON");
                    mat.DisableKeyword("_ALPHABLEND_ON");
                    mat.EnableKeyword("_ALPHAPREMULTIPLY_ON");
                    mat.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.One);
                    mat.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
#endif
                }
            }

            // If the material was set up by meshsync but doesn't need that anymore, revert it to a standard material:
            if (!usingOverride && Array.IndexOf(mat.shaderKeywords, "MESHSYNC_OVERRIDE") >= 0) {
                mat.CopyPropertiesFromMaterial(new Material(shader));
            }
        }
    }
}
