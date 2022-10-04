using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync {
    partial class BaseMeshSync {
        static readonly string[] textureNames = {
            _MetallicGlossMap,
            _BaseMap
        };

        void SaveMaterialRenderTexturesToAssetdatabase() {
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

                        var savePath = Path.Combine(GetAssetsFolder(), $"{mat.name}_{textureName}_baked.png");
                        TextureData.WriteToFile(savePath, texture.EncodeToPNG());

                        if (string.IsNullOrEmpty(AssetDatabase.AssetPathToGUID(savePath))) {
                            AssetDatabase.ImportAsset(savePath);
                        }

                        if (textureName == _MetallicGlossMap) {
                            TextureImporter importer = (TextureImporter)AssetImporter.GetAtPath(savePath);
                            if (importer != null) {
                                importer.sRGBTexture = false;
                            }
                        }

                        var savedTexture = AssetDatabase.LoadAssetAtPath<Texture2D>(savePath);
                    
                        mat.SetTexture(textureName, savedTexture);
                    }
                }
            }
        }
    }
}
