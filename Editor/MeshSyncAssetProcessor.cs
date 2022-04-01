using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEditor.Experimental;
using UnityEngine;

namespace Unity.MeshSync
{
    public class MeshSyncAssetProcessor : AssetsModifiedProcessor
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
            var servers = GameObject.FindObjectsOfType<MeshSyncServer>();
            foreach (var path in m_added)
            {
                // Get the added asset
                var asset = AssetDatabase.LoadAssetAtPath<GameObject>(path);
                if (asset == null)
                    continue;

                // Find the instance of the asset.
                foreach (var server in servers)
                {
                    var children = server.transform.GetComponentsInChildren<Transform>(true);
                    foreach(var child in children)
                    {
                        var instancePrefab = PrefabUtility.GetCorrespondingObjectFromSourceAtPath(child, path);
                        if (instancePrefab == null)
                            continue;

                        var basePath = server.GetAssetsFolder();
                    
                        var instanceChildren = 
                            child.GetComponentsInChildren<Renderer>();
                        
                        var assetChildren = asset.GetComponentsInChildren<Renderer>();

                        for (var i = 0; i < instanceChildren.Length; i++)
                        {
                            var instanceChild = instanceChildren[i];
                            var assetChild = assetChildren[i];
                        
                            UpdateMesh(instanceChild, assetChild, basePath);
                            UpdateMaterial(instanceChild, assetChild, basePath);
                        }
                    
                        // Only need to do this once per prefab
                        break;
                    }
                   
                }
            }
            
            m_added.Clear();
            
            AssetDatabase.SaveAssets();
        }

        private void UpdateMaterial(Renderer instance, Renderer asset, string basePath)
        {
            // save the material referenced in the instance
            var savedMaterials = SaveMaterials(instance, basePath);
            asset.sharedMaterials = savedMaterials;
        }
        
        private void UpdateMesh(Renderer instance, Renderer asset, string basePath)
        {
            // save the mesh referenced in the instance
            var savedMesh = SaveMesh(instance, basePath);
            SetMesh(asset, savedMesh);
        }

        private Mesh GetMesh(Renderer renderer)
        {
            if (renderer is MeshRenderer meshRenderer)
            {
                var filter = renderer.gameObject.GetComponent<MeshFilter>();
                return filter.sharedMesh;
            }

            if (renderer is SkinnedMeshRenderer skinnedMeshRenderer)
            {
                return skinnedMeshRenderer.sharedMesh;
            }

            return null;
        }

        private void SetMesh(Renderer renderer, Mesh mesh)
        {
            if (renderer is MeshRenderer meshRenderer)
            {
                var filter = renderer.gameObject.GetComponent<MeshFilter>();
                filter.sharedMesh = mesh;
                return;
            }

            if (renderer is SkinnedMeshRenderer skinnedMeshRenderer)
            {
                skinnedMeshRenderer.sharedMesh = mesh;
                return;
            }
        }
        
        private Material[] SaveMaterials(Renderer renderer, string basePath)
        {
            var materials = renderer.sharedMaterials;
            var result = new Material[materials.Length];
            for (var i = 0; i < materials.Length; i++)
            {
                CreateDirectoryIfNotExists(basePath);
                
                var nameGenerator = new Misc.UniqueNameGenerator();

                var material = materials[i];
                var dstPath = $"{basePath}/{nameGenerator.Gen(material.name)}.mat";
                
                 result[i] = Misc.OverwriteOrCreateAsset(material, dstPath);
            }

            
            return result;
        }

        private Mesh SaveMesh(Renderer renderer, string basePath)
        {
            var mesh = GetMesh(renderer);

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