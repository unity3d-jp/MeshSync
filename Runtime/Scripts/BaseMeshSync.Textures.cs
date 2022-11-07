using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.Assertions;

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync {
    partial class BaseMeshSync {
#if UNITY_EDITOR
        /// <summary>
        /// Names of all maps that could be baked to a render texture and might need to be saved to the asset database.
        /// </summary>
        static readonly int[] textureNameIDs = {
            MeshSyncConstants._MetallicGlossMap,
            MeshSyncConstants._BaseMap,
            MeshSyncConstants._MaskMap
        };

        static string nameFromID(int id) {
            if (id == MeshSyncConstants._MetallicGlossMap)
                return "metallic";
            if (id == MeshSyncConstants._BaseMap)
                return "basemap";
            if (id == MeshSyncConstants._MaskMap)
                return "maskmap";

            return id.ToString();
        }

        private List<Tuple<Material, int>> pendingMaterialUpdates = new List<Tuple<Material, int>>();

        /// <summary>
        /// Returns the path where the given texture should be saved.
        /// </summary>
        /// <param name="mat">Material that owns the texture</param>
        /// <param name="textureName">Name of the texture in the material</param>
        /// <returns></returns>
        private string GetSavePath(Material mat, int textureNameID) {
            string textureName = nameFromID(textureNameID);
            return Path.Combine(GetAssetsFolder(), $"{mat.name}_{textureName}_baked.png");
        }

        /// <summary>
        /// Saves any render targets to the AssetDatabase as textures.
        /// </summary>
        private void SaveMaterialRenderTexturesToAssetDatabase() {
            // Check if there are render textures that need to be saved to the asset database:
            foreach (MaterialHolder materialHolder in materialList) {
                foreach (int textureNameID in textureNameIDs) {
                    var mat = materialHolder.material;

                    if (mat == null || !mat.HasProperty(textureNameID)) {
                        continue;
                    }

                    var tex = mat.GetTexture(textureNameID);
                    if (tex == null) {
                        continue;
                    }

                    if (tex is RenderTexture renderTarget) {
                        using (var texture = new Texture2DDisposable(
                                   new Texture2D(renderTarget.width, renderTarget.height,
                                   UnityEngine.TextureFormat.RGBA32, true))) {
                            texture.ReadFromRenderTexture(renderTarget);

                            var savePath = GetSavePath(mat, textureNameID);
                            TextureData.WriteToFile(savePath, texture.Texture.EncodeToPNG());
                        }

                        // Setting the texture can fail if the asset database has not imported it yet.
                        // If that happens, save it for later and try again:
                        if (!SetSerializedTextureForMaterial(mat, textureNameID)) {
                            pendingMaterialUpdates.Add(new Tuple<Material, int>(mat, textureNameID));

                            // Set the texture on the material to null for now so the render texture
                            // is not saved to file again if this gets called again before it was
                            // set to the texture in the asset library:
                            mat.SetTextureSafe(textureNameID, null);
                        }

                        renderTarget.Release();
                    }
                }
            }
        }

        /// <summary>
        /// Sets the texture on the material if it exists in the AssetDatabase, otherwise it schedules a delayed call to try again when the asset might be imported.
        /// </summary>
        /// <param name="mat">Material to set the texture on</param>
        /// <param name="textureName">Name of the texture in the material</param>
        /// <returns></returns>
        private bool SetSerializedTextureForMaterial(Material mat, int textureNameID) {
            var savePath = GetSavePath(mat, textureNameID);

            // Ensure the texture is up to date:
            AssetDatabase.ImportAsset(savePath);
            Texture2D savedTexture;

            // LoadAssetAtPath can throw an exception if it's called during a domain backup but we have no way to detect that:
            try {
                savedTexture = AssetDatabase.LoadAssetAtPath<Texture2D>(savePath);
                if (savedTexture == null) {
                    EditorApplication.delayCall -= UpdatePendingMaterials;
                    EditorApplication.delayCall += UpdatePendingMaterials;

                    return false;
                }
            }
            catch (UnityException) {
                return false;
            }

            mat.SetTexture(textureNameID, savedTexture);

            if (textureNameID == MeshSyncConstants._MetallicGlossMap ||
                textureNameID == MeshSyncConstants._MaskMap) {
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

        /// <summary>
        /// Tries to apply any pending material updates.
        /// </summary>
        private void UpdatePendingMaterials() {
            for (int i = pendingMaterialUpdates.Count - 1; i >= 0; i--) {
                Tuple<Material, int> pendingMaterial = pendingMaterialUpdates[i];
                if (SetSerializedTextureForMaterial(pendingMaterial.Item1, pendingMaterial.Item2)) {
                    pendingMaterialUpdates.RemoveAt(i);
                }
            }
        }
#endif

        /// <summary>
        /// Returns the default shader for the active render pipeline.
        /// </summary>
        private static Shader GetStandardShader() {
#if AT_USE_HDRP
            return Shader.Find("HDRP/Lit");
#elif AT_USE_URP
            return Shader.Find("Universal Render Pipeline/Lit");
#else
            return Shader.Find("Standard");
#endif
        }

        /// <summary>
        /// Sets the required shader for the given material.
        /// </summary>
        /// <param name="mat">Material to set the shader on</param>
        /// <param name="shaderName">The name of the shader passed by the DCC tool</param>
        private static void UpdateShader(ref Material mat, string shaderName) {
            Shader shader = null;
            if (!string.IsNullOrEmpty(shaderName)) {
                shader = Shader.Find(shaderName);
            }

            bool shaderExists = shader != null;

            if (shader == null) {
                shader = GetStandardShader();
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
                    SetupGlassShader(mat);
                    usingOverride = true;
                }
            }

            // If the material was set up by meshsync but doesn't need that anymore, revert it to a standard material:
            if (!usingOverride && Array.IndexOf(mat.shaderKeywords, MeshSyncConstants.MESHSYNC_OVERRIDE) >= 0) {
                mat.CopyPropertiesFromMaterial(new Material(shader));
            }
        }

        /// <summary>
        /// Sets the material up to represent a glass shader from DCC tools.
        /// </summary>
        /// <param name="mat">The material to change.</param>
        private static void SetupGlassShader(Material mat) {
            mat.EnableKeyword(MeshSyncConstants.MESHSYNC_OVERRIDE);

            mat.EnableKeyword(MeshSyncConstants._SURFACE_TYPE_TRANSPARENT);
            mat.SetOverrideTag(MeshSyncConstants.RenderType, MeshSyncConstants.Transparent);
            mat.SetFloat(MeshSyncConstants._Surface, 1);

#if AT_USE_HDRP
            SetupGlass_HRDP(mat);
#elif AT_USE_URP
            SetupGlass_URP(mat);
#else
            SetupGlass_Standard(mat);
#endif
        }

        private static void SetupGlass_HRDP(Material mat) {
            mat.EnableKeyword(MeshSyncConstants._ENABLE_FOG_ON_TRANSPARENT);

            mat.renderQueue = 3000;

            mat.SetFloat(MeshSyncConstants._SurfaceType, 1);
            mat.SetInt(MeshSyncConstants._ZWrite, 0);
            mat.SetInt(MeshSyncConstants._SrcBlend, (int)UnityEngine.Rendering.BlendMode.One);
            mat.SetInt(MeshSyncConstants._DstBlend, (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
            mat.SetFloat(MeshSyncConstants._AlphaSrcBlend, (int)UnityEngine.Rendering.BlendMode.One);
            mat.SetFloat(MeshSyncConstants._AlphaDstBlend, (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
            mat.SetFloat(MeshSyncConstants._ZTestDepthEqualForOpaque, 4);
        }

        private static void SetupGlass_URP(Material mat) {
            mat.SetFloat(MeshSyncConstants._Blend, 1);
            mat.DisableKeyword(MeshSyncConstants._ALPHATEST_ON);
            mat.DisableKeyword(MeshSyncConstants._ALPHABLEND_ON);
            mat.EnableKeyword(MeshSyncConstants._ALPHAPREMULTIPLY_ON);
            mat.SetInt(MeshSyncConstants._SrcBlend, (int)UnityEngine.Rendering.BlendMode.One);
            mat.SetInt(MeshSyncConstants._DstBlend, (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
        }

        private static void SetupGlass_Standard(Material mat) {
            mat.SetFloat(MeshSyncConstants._Mode, 3);
            mat.DisableKeyword(MeshSyncConstants._ALPHATEST_ON);
            mat.DisableKeyword(MeshSyncConstants._ALPHABLEND_ON);
            mat.EnableKeyword(MeshSyncConstants._ALPHAPREMULTIPLY_ON);
            mat.SetInt(MeshSyncConstants._SrcBlend, (int)UnityEngine.Rendering.BlendMode.One);
            mat.SetInt(MeshSyncConstants._DstBlend, (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
        }
    }
}
