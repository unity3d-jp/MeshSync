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
            DryRunWithoutSyncAndSave,
            SyncButDoNotSave,
            SaveAsPrefab
        }

        MeshSyncServer server => variantExporter.Server;
        string SavePath => Path.Combine("Assets", variantExporter.SaveFile);

        protected VariantExporter variantExporter;

        protected object[] currentSettings;

        public int counter;

        SaveMode saveMode = SaveMode.SaveAsPrefab;

        public ReadOnlyCollection<PropertyInfoDataWrapper> propertyInfos => variantExporter.EnabledProperties;

        
        public abstract int PermutationCount
        {
            get;
        }

        public PermutationRunnerBase(VariantExporter variantExporter)
        {
            this.variantExporter = variantExporter;

            currentSettings = new object[variantExporter.enabledProperties.Count];
        }

        protected abstract IEnumerator ExecutePermutations(int propIdx);

        IEnumerator Save()
        {
            var permutationCount = variantExporter.PermutationCount;
            if (EditorUtility.DisplayCancelableProgressBar("Exporting", $"Variant {counter + 1}/{permutationCount}", (float)counter / permutationCount))
            {
                variantExporter.StopExport();
            }

            if (saveMode != SaveMode.DryRunWithoutSyncAndSave)
            {
                // Set all properties here at once so they can be sent to blender together:
                for (int i = 0; i < propertyInfos.Count; i++)
                {
                    var prop = propertyInfos[i];
                    prop.NewValue = currentSettings[i];
                }

                var s = new StringBuilder();
                foreach (var prop in propertyInfos)
                {
                    s.AppendLine(prop.ToString());
                }

                Debug.Log(s);

                yield return new WaitForMeshSync(server);

                switch (saveMode)
                {
                    case SaveMode.SaveAsPrefab:
                        yield return SavePrefab();
                        break;

                    case SaveMode.SyncButDoNotSave:
                        break;

                    default:
                        throw new NotImplementedException($"save mode {saveMode} not implemented");
                }
            }

            counter++;

            if (counter >= permutationCount)
            {
                Debug.Log($"Finished exporting {variantExporter.SaveFile}.");

                variantExporter.StopExport();
            }

            yield break;
        }

        IEnumerator SavePrefab()
        {
            var outputFilePath = SavePath;

            if (!Directory.Exists(outputFilePath))
            {
                Directory.CreateDirectory(outputFilePath);
            }

            var prefabSave = server.gameObject; // Instantiate(server.gameObject);

            var commonAssetStorePath = Path.Combine(outputFilePath, $"{variantExporter.SaveFile}_assets.asset");

            outputFilePath = Path.Combine(outputFilePath, $"{variantExporter.SaveFile}_{counter}.prefab");
            PrefabUtility.SaveAsPrefabAsset(prefabSave, outputFilePath);

            var sharedAssets = new List<UnityEngine.Object>(AssetDatabase.LoadAllAssetsAtPath(commonAssetStorePath));

            var meshFilters = prefabSave.GetComponentsInChildren<MeshFilter>(true);
            foreach (var child in meshFilters)
            {
                try
                {
                    bool useSubAsset = true;

                    if (useSubAsset)
                    {
                        bool alreadySaved = false;
                        foreach (var sharedAsset in sharedAssets)
                        {
                            if (sharedAsset.name == child.sharedMesh.name)
                            {
                                alreadySaved = true;
                                break;
                            }
                        }

                        if (alreadySaved)
                        {
                            continue;
                        }

                        //var asset = child.sharedMesh;
                        var asset = UnityEngine.Object.Instantiate(child.sharedMesh);
                        asset.name = child.sharedMesh.name;

                        if (!File.Exists(commonAssetStorePath))
                        {
                            AssetDatabase.CreateAsset(asset, commonAssetStorePath);
                        }
                        else
                        {
                            AssetDatabase.AddObjectToAsset(asset, commonAssetStorePath);
                        }

                        sharedAssets.Add(asset);
                    }
                    else
                    {
                        AssetDatabase.AddObjectToAsset(child.sharedMesh, outputFilePath);
                    }
                }
                catch (Exception ex)
                {
                    Debug.LogException(ex);
                }

                // Probably don't need this:
                AssetDatabase.SaveAssets();


                //var renderers = prefabSave.GetComponentsInChildren<Renderer>(true);
                //foreach (var child in renderers)
                //{
                //    try
                //    {
                //        foreach (var mat in child.sharedMaterials)
                //        {
                //            AssetDatabase.AddObjectToAsset(mat, outputFilePath);
                //        }
                //    }
                //    catch (Exception ex)
                //    {
                //        Debug.LogException(ex);
                //        //baker.StopBake();
                //    }
                //}
            }

            //DestroyImmediate(prefabSave); 

            yield break;
        }

        public IEnumerator Start()
        {
            var outputFilePath = SavePath;

            // For now delete the folder if it exists, maybe we'd want to resume where we left off if there are files in there?
            if (Directory.Exists(outputFilePath))
            {
                Directory.Delete(outputFilePath, true);

                AssetDatabase.Refresh();
            }

            yield return Next(-1);
        }

        protected IEnumerator Next(int propIdx)
        {
            propIdx++;

            if (propIdx == propertyInfos.Count)
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
