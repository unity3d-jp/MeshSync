using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEditor.Experimental;
using UnityEngine;

namespace Unity.MeshSync
{
    public class InstanceRendererAssetProcessor : AssetsModifiedProcessor
    {
        private List<string> m_added = new List<string>();
            
        protected override void OnAssetsModified(string[] changedAssets, string[] addedAssets, string[] deletedAssets,
            AssetMoveInfo[] movedAssets)
        {
            m_added.AddRange(addedAssets);
            EditorApplication.delayCall += DelayCall;
        }

        
        private void DelayCall()
        {
            var gos = GameObject.FindObjectsOfType<MeshSyncInstanceRenderer>();
            foreach (var path in m_added)
            {
                // Get the added asset
                var asset = AssetDatabase.LoadAssetAtPath<MeshSyncInstanceRenderer>(path);
                if (asset == null)
                    continue;
                    
                // Find the instance of the asset.
                foreach (var instance in gos)
                {
                    var instancePrefab = PrefabUtility.GetCorrespondingObjectFromSourceAtPath(instance, path);
                    if (instancePrefab != asset)
                        continue;

                    var instanceChildren = 
                        instance.GetComponentsInChildren<MeshSyncInstanceRenderer>();
                    var assetChildren = asset.GetComponentsInChildren<MeshSyncInstanceRenderer>();

                    for (var i = 0; i < instanceChildren.Length; i++)
                    {
                        var instanceChild = instanceChildren[i];
                        var assetChild = assetChildren[i];
                        
                        UpdateMesh(instanceChild, assetChild);
                        UpdateMaterial(instanceChild, assetChild);
                    }
                    
                    // Only need to do this once per prefab
                    break;
                }
            }
            
            m_added.Clear();
            
            AssetDatabase.SaveAssets();
        }

        private void UpdateMaterial(MeshSyncInstanceRenderer instance, MeshSyncInstanceRenderer asset)
        {
            // save the material referenced in the instance
            var savedMaterials = SaveMaterials(instance);

            if (asset.TryGetComponent(out Renderer r))
            {
                r.sharedMaterials = savedMaterials;
            }
        }
        
        private void UpdateMesh(MeshSyncInstanceRenderer instance, MeshSyncInstanceRenderer asset)
        {
            // save the mesh referenced in the instance
            var savedMesh = SaveMesh(instance);

            // assign the saved mesh to the asset
            if (asset.TryGetComponent(out SkinnedMeshRenderer smr))
            {
                smr.sharedMesh = savedMesh;
            }
            else if (asset.TryGetComponent(out MeshFilter mf))
            {
                mf.sharedMesh = savedMesh;
            }
        }
        
        private Material[] SaveMaterials(MeshSyncInstanceRenderer renderer)
        {
            var materials = renderer.m_renderingInfo.Materials;
            var result = new Material[materials.Length];
            for (var i = 0; i < materials.Length; i++)
            {
                var server = renderer.m_server;
                var basePath = server.GetAssetsFolder();

                CreateDirectoryIfNotExists(basePath);
                
                var nameGenerator = new Misc.UniqueNameGenerator();

                var material = materials[i];
                var dstPath = $"{basePath}/{nameGenerator.Gen(material.name)}.mat";
                 result[i] = Misc.OverwriteOrCreateAsset(material, dstPath);
            }

            return result;
        }

        private Mesh SaveMesh(MeshSyncInstanceRenderer renderer)
        {
            var mesh = renderer.m_renderingInfo.Mesh;
            var server = renderer.m_server;
            var basePath = server.GetAssetsFolder();
            

            CreateDirectoryIfNotExists(basePath);

            var nameGenerator = new Misc.UniqueNameGenerator();
            
            var dstPath = $"{basePath}/{nameGenerator.Gen(mesh.name)}.asset";
            return Misc.OverwriteOrCreateAsset(mesh, dstPath);
        }
        
        private void CreateDirectoryIfNotExists(string path) {
            if (Directory.Exists(path))
                return;
            
            Directory.CreateDirectory(path);
            AssetDatabase.Refresh();
        }
    }
}