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
                Debug.Log($"Finished exporting {regenerator.SaveFile}.");

                regenerator.StopExport();
                yield break;
            }

            yield break;
        }

        static T SaveAsset<T>(T asset, string path) where T : UnityEngine.Object
        {
            T loadedAsset = AssetDatabase.LoadAssetAtPath<T>(path);
            if (loadedAsset != null)
            {
                EditorUtility.CopySerialized(asset, loadedAsset);
                return loadedAsset;
            }
            else
            {
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
                else
                {
                    try
                    {
                        AssetDatabase.CreateAsset(asset, path);
                    }
                    catch (Exception ex)
                    {

                    }
                }
            }

            return asset;
        }

        static GameObject SavePrefabAssets(GameObject prefab, string prefabAssetStorePath, string prefabSavePath)
        {
            var meshFilters = prefab.GetComponentsInChildren<MeshFilter>();
            foreach (var meshFilter in meshFilters)
            {
                var mesh = meshFilter.sharedMesh;
                if (mesh != null)
                {
                    string dstPath = Path.Combine(prefabAssetStorePath, $"{mesh.name}.asset");

                    meshFilter.sharedMesh = SaveAsset(mesh, dstPath);
                }
            }

            if (prefabSavePath != null)
            {
                return PrefabUtility.SaveAsPrefabAsset(prefab, prefabSavePath);
            }

            return null;
        }

        IEnumerator SavePrefab()
        {
            var outputFilePath = regenerator.SavePath;

            if (!Directory.Exists(outputFilePath))
            {
                Directory.CreateDirectory(outputFilePath);
            }

            if (Regenerator.StopAssetEditing)
            {
                AssetDatabase.StopAssetEditing();
            }

            // Make a copy, apply the prefab-specific save location and save any changed assets to that:
            var prefabSavePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_{counter}.prefab");

            var prefabAssetStorePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_{counter}_assets");

            if (Directory.Exists(prefabAssetStorePath))
            {
                AssetDatabase.DeleteAsset(prefabAssetStorePath);
            }

            Directory.CreateDirectory(prefabAssetStorePath);

            //var p = server.GetAssetsFolder();
            //server.SetAssetsFolder(prefabAssetStorePath);

            //server.ExportMeshes();

            var prefabSave = SavePrefabAssets(server.gameObject, prefabAssetStorePath, prefabSavePath);

            //var commonAssetStorePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_assets");
            //SavePrefabAssets(server.gameObject, commonAssetStorePath, null);

            //server.SetAssetsFolder(p);

            PurgePrefab(prefabSave);


            var prefabServer = prefabSave.GetComponent<MeshSyncServer>();

            if (false)
            {
                prefabServer.CopySettingsFrom(server);
                prefabServer.SetAssetsFolder(prefabAssetStorePath);

                prefabServer.ExportMeshes();
            }
            UnityEngine.Object.DestroyImmediate(prefabServer, true);

            //AssetDatabase.SaveAssets();

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

            //AssetDatabase.SaveAssets();

            // Ensure the object is clean, delete children and update from blender:
            regenerator.Server.ClearInstancePrefabs();

            yield return new WaitForMeshSync(regenerator);

            if (Regenerator.StopAssetEditing)
            {
                AssetDatabase.StartAssetEditing();
            }

            //regenerator.StopExport();

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

        public IEnumerator Start()
        {
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

            var commonAssetStorePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_assets");

            regenerator.Server.SetAssetsFolder(commonAssetStorePath);

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
    }
}
