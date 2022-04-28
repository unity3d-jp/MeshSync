using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Text;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.VariantExport
{
    abstract class PermutationRunnerBase
    {
        enum SaveMode
        {
            SaveAsPrefab
        }

        protected MeshSyncServer server => regenerator.Server;

        protected Regenerator regenerator;

        public int counter;

        SaveMode saveMode = SaveMode.SaveAsPrefab;

        List<string> savedAssets = new List<string>();
        //Dictionary<string, string> savedAssetsReferences = new Dictionary<string, string>();


        string CommonAssetStorePath
        {
            get
            {
                return Path.Combine(regenerator.SavePath, $"{regenerator.SaveFile}_assets");
            }
        }

        //public ReadOnlyCollection<PropertyInfoDataWrapper> propertyInfos => .EnabledProperties;

        public abstract int VariantCount
        {
            get;
        }

        public int TotalPermutationCount
        {
            get
            {
                int total = 1;

                foreach (var prop in server.propertyInfos)
                {
                    int range = 1;

                    // TODO: Make this work with float ranges:
                    switch (prop.type)
                    {
                        case PropertyInfoData.Type.Int:
                            range = (int)(prop.max - prop.min) + 1;
                            break;
                    }

                    total *= range;
                }

                return total;
            }
        }

        public PermutationRunnerBase(Regenerator regenerator)
        {
            this.regenerator = regenerator;
        }

        public bool CheckForCancellation()
        {
            var permutationCount = VariantCount;

            if (EditorUtility.DisplayCancelableProgressBar("Exporting", $"Variant {counter + 1}/{permutationCount}", (float)counter / permutationCount))
            {
                regenerator.StopExport();
                return true;
            }

            return false;
        }

        protected abstract IEnumerator ExecutePermutations(int propIdx);

        public IEnumerator Start()
        {
            savedAssets.Clear();

            if (CheckForCancellation())
            {
                yield break;
            }

            var outputFilePath = regenerator.SavePath;

            if (Directory.Exists(outputFilePath))
            {
                foreach (var prefabFile in Directory.EnumerateFiles(outputFilePath, "*.prefab"))
                {
                    AssetDatabase.DeleteAsset(prefabFile);
                    //File.Delete(prefabFile);
                }

                AssetDatabase.Refresh();
            }

            // Make sure server is at origin so any saved prefabs are at origin too:
            server.transform.localPosition = Vector3.zero;

            regenerator.Server.SetAssetsFolder(CommonAssetStorePath);

            //regenerator.Server.ExportMeshes();
            regenerator.Server.ExportMaterials();

            // Ensure the object is clean, delete children and update from blender:
            regenerator.Server.ClearInstancePrefabs();

            yield return new WaitForMeshSync(regenerator);

            yield return Next(-1);
        }

        protected IEnumerator Next(int propIdx)
        {
            propIdx++;

            if (propIdx == server.propertyInfos.Count)
            {
                yield return Save();
            }
            else
            {
                yield return ExecutePermutations(propIdx);
            }
        }

        protected IEnumerator Save()
        {
            if (CheckForCancellation())
            {
                yield break;
            }

            var s = new StringBuilder();
            foreach (var prop in server.propertyInfos)
            {
                s.AppendLine(prop.ToString());
            }

            //Debug.Log(s);

            yield return new WaitForMeshSync(regenerator);

            switch (saveMode)
            {
                case SaveMode.SaveAsPrefab:
                    yield return SavePrefab();
                    break;

                default:
                    throw new NotImplementedException($"save mode {saveMode} not implemented");
            }

            counter++;

            if (counter >= VariantCount)
            {
                Finished();

                Debug.Log($"Finished exporting {regenerator.SaveFile}.");

                regenerator.StopExport();
                yield break;
            }

            yield break;
        }

        T SaveAsset<T>(T asset, string path) where T : UnityEngine.Object
        {
            if (savedAssets.Contains(path))
            {
                //Debug.LogError($"Already had {path}");

                var existingAsset = AssetDatabase.LoadAssetAtPath<T>(path);
                if (existingAsset == null)
                {
                    Debug.LogError("Asset not found!");
                }

                return existingAsset;
            }

            savedAssets.Add(path);

            // If it's an existing asset but the path doesn't match, make a copy of it for the new path:
            if (AssetDatabase.Contains(asset))
            {
                var assetPath = Path.GetFullPath(AssetDatabase.GetAssetPath(asset));
                var fullPath = Path.GetFullPath(path);

                if (assetPath != fullPath)
                {
                    var newAsset = UnityEngine.Object.Instantiate(asset);
                    newAsset.name = asset.name;
                    asset = newAsset;
                }
            }

            // to keep meta, rewrite the existing one if already exists.
            T loadedAsset = AssetDatabase.LoadAssetAtPath<T>(path);
            if (loadedAsset != null)
            {
                //Debug.LogError($"Existed {path}");
                EditorUtility.CopySerialized(asset, loadedAsset);
                return loadedAsset;
            }

            AssetDatabase.CreateAsset(asset, path);
            //Debug.LogError($"Created {path}");
            return asset;
        }

        GameObject SavePrefabAssets(GameObject prefab, string prefabAssetStorePath, string prefabSavePath, bool exportHidden)
        {
            // TODO: Add material support:
            bool saveMaterials = false;

            var meshFilters = prefab.GetComponentsInChildren<MeshFilter>(exportHidden);
            foreach (var meshFilter in meshFilters)
            {
                var mesh = meshFilter.sharedMesh;
                if (mesh != null)
                {
                    string dstPath = Path.Combine(prefabAssetStorePath, $"{mesh.name}.asset");

                    meshFilter.sharedMesh = SaveAsset(mesh, dstPath);
                }
            }

            if (saveMaterials)
            {
                var meshRenderers = prefab.GetComponentsInChildren<MeshRenderer>(exportHidden);
                foreach (var meshRenderer in meshRenderers)
                {
                    var material = meshRenderer.sharedMaterial;
                    if (material != null)
                    {
                        string dstPath = Path.Combine(prefabAssetStorePath, $"{material.name}.mat");

                        meshRenderer.sharedMaterial = SaveAsset(material, dstPath);
                    }
                    else
                    {
                        Debug.LogError($"No material on {meshRenderer.name}");
                        regenerator.StopExport();
                        throw new Exception("");
                    }
                }
            }

            var skinnedMeshRenderers = prefab.GetComponentsInChildren<SkinnedMeshRenderer>(exportHidden);
            foreach (var skinnedMeshRenderer in skinnedMeshRenderers)
            {
                if (saveMaterials)
                {
                    var material = skinnedMeshRenderer.sharedMaterial;
                    if (material != null)
                    {
                        string dstPath = Path.Combine(prefabAssetStorePath, $"{material.name}.mat");

                        skinnedMeshRenderer.sharedMaterial = SaveAsset(material, dstPath);
                    }
                }

                var mesh = skinnedMeshRenderer.sharedMesh;
                if (mesh != null)
                {
                    string dstPath = Path.Combine(prefabAssetStorePath, $"{mesh.name}.asset");

                    skinnedMeshRenderer.sharedMesh = SaveAsset(mesh, dstPath);
                }
            }

            return PrefabUtility.SaveAsPrefabAsset(prefab, prefabSavePath);
        }

        IEnumerator SavePrefab()
        {
            var outputFilePath = regenerator.SavePath;

            if (!Directory.Exists(outputFilePath))
            {
                Directory.CreateDirectory(outputFilePath);
            }

            // Make a copy, apply the prefab-specific save location and save any changed assets to that:
            var prefabSavePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_{counter}.prefab");

            var prefabAssetStorePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_{counter}_assets");

            if (Directory.Exists(prefabAssetStorePath))
            {
                AssetDatabase.DeleteAsset(prefabAssetStorePath);
            }

            Directory.CreateDirectory(prefabAssetStorePath);

            var prefabSave = SavePrefabAssets(server.gameObject,
                prefabAssetStorePath,
                prefabSavePath,
                exportHidden: server.InstanceHandling == BaseMeshSync.InstanceHandlingType.InstanceRenderer);

            PurgePrefab(prefabSave);

            var prefabServer = prefabSave.GetComponent<MeshSyncServer>();

            UnityEngine.Object.DestroyImmediate(prefabServer, true);

            // If there are no assets specific to this prefab, delete the empty folder:
            if (Directory.GetFiles(prefabAssetStorePath).Length == 0)
            {
                AssetDatabase.DeleteAsset(prefabAssetStorePath);
            }
            else
            {
                //AssetDatabase.SaveAssets();
            }

            PrefabUtility.SavePrefabAsset(prefabSave);

            // Ensure the object is clean, delete children and update from blender:
            regenerator.Server.ClearInstancePrefabs();

            yield return new WaitForMeshSync(regenerator);

            yield break;
        }

        void PurgePrefab(GameObject prefab)
        {
            //return;
            // Find disabled gameobjects and get rid of them.
            // Build a list of them first so we don't get exceptions for things that were already destroyed.
            var allChildren = prefab.GetComponentsInChildren<Transform>(true);
            List<GameObject> childrenToDelete = new List<GameObject>();
            foreach (var child in allChildren)
            {
                if (!child.gameObject.activeSelf &&
                    server.InstanceHandling != BaseMeshSync.InstanceHandlingType.InstanceRenderer)
                {
                    childrenToDelete.Add(child.gameObject);
                }
                else
                {
                    // Check for empty gameobjects with all empty children:
                    bool valid = false;
                    var components = child.GetComponentsInChildren<Component>();
                    foreach (var component in components)
                    {
                        if (!(component is Transform))
                        {
                            valid = true;
                            break;
                        }
                    }

                    if (!valid)
                    {
                        childrenToDelete.Add(child.gameObject);
                    }
                }
            }

            foreach (var child in childrenToDelete)
            {
                UnityEngine.Object.DestroyImmediate(child, true);
            }
        }

        void Finished()
        {
            var matchingAssets = new Dictionary<string, List<string>>();

            var processedAssets = new HashSet<string>();

            // Find duplicates:
            for (int i = 0; i < savedAssets.Count; i++)
            {
                string assetPath1 = savedAssets[i];

                if (processedAssets.Contains(assetPath1))
                {
                    continue;
                }

                var assetFilename1 = Path.GetFileName(assetPath1);

                // Check if the same asset name exists somewhere else:
                for (int j = i + 1; j < savedAssets.Count; j++)
                {
                    string assetPath2 = savedAssets[j];
                    var assetFilename2 = Path.GetFileName(assetPath2);

                    if (assetFilename1 == assetFilename2)
                    {
                        // Compare contents of the file:
                        var assetContent1 = File.ReadAllText(assetPath1);
                        var assetContent2 = File.ReadAllText(assetPath2);

                        if (assetContent1 == assetContent2)
                        {
                            if (!matchingAssets.TryGetValue(assetPath1, out var assetList))
                            {
                                assetList = new List<string>() { assetPath1 };
                                matchingAssets.Add(assetPath1, assetList);
                            }

                            assetList.Add(assetPath2);

                            processedAssets.Add(assetPath2);
                        }
                    }
                }
            }

            var allFilesInDir = Directory.GetFiles(regenerator.SavePath, "*.prefab");

            try
            {
                int counter = 0;

                void RemoveAsset(string assetPath)
                {
                    AssetDatabase.DeleteAsset(assetPath);
                    var dir = Path.GetDirectoryName(assetPath);
                    if (Directory.Exists(dir) && Directory.GetFiles(dir).Length == 0)
                    {
                        //AssetDatabase.Refresh();
                        AssetDatabase.DeleteAsset(dir);
                    }
                }

                foreach (var kvp in matchingAssets)
                {
                    EditorUtility.DisplayProgressBar("Making assets shared", kvp.Key, (float)counter / matchingAssets.Count);

                    var assetPath = kvp.Key;

                    // Save to shared folder:
                    var newPath = Path.Combine(CommonAssetStorePath, Path.GetFileName(assetPath));

                    if (Directory.Exists(newPath))
                    {
                        EditorUtility.CopySerialized(AssetDatabase.LoadAssetAtPath<UnityEngine.Object>(assetPath), AssetDatabase.LoadAssetAtPath<UnityEngine.Object>(newPath));
                    }
                    else
                    {
                        AssetDatabase.CopyAsset(assetPath, newPath);
                    }

                    RemoveAsset(assetPath);

                    var sharedAssetGUID = $"guid: {AssetDatabase.GUIDFromAssetPath(newPath)}";

                    foreach (var duplicateAsset in kvp.Value)
                    {
                        var duplicateAssetGUID = $"guid: {AssetDatabase.GUIDFromAssetPath(duplicateAsset)}";

                        foreach (var file in allFilesInDir)
                        {
                            string contents = File.ReadAllText(file);

                            var newContents = contents.Replace(duplicateAssetGUID, sharedAssetGUID);

                            if (newContents != contents)
                            {
                                File.WriteAllText(file, newContents);

                                Debug.Log($"Changed GUID in {file} from {duplicateAssetGUID} to {sharedAssetGUID} ({Path.GetFileName(assetPath)})");
                            }
                        }

                        RemoveAsset(duplicateAsset);
                    }

                    counter++;
                }

                //AssetDatabase.Refresh();
            }
            catch (Exception ex)
            {
                Debug.LogException(ex);
            }
            finally
            {
                EditorUtility.ClearProgressBar();

                AssetDatabase.Refresh();
            }
        }
    }
}
