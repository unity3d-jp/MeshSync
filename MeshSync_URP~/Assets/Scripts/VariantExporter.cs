using System.Collections;
using System.Collections.Generic;
using System.Text;
using Unity.MeshSync;
using UnityEditor;
using UnityEngine;
using Unity.EditorCoroutines.Editor;
using System;
using System.IO;
using System.Threading;
using System.Collections.ObjectModel;

namespace Unity.MeshSync
{
    [ExecuteAlways]
    public class VariantExporter : MonoBehaviour
    {
        public enum ExportModeSetting
        {
            RegenerateEverything,
            RegenerateOnlyExisting,
        }

        [SerializeField]
        private MeshSyncServer server;

        public string SaveFile;

        public ExportModeSetting ExportMode;

        public bool IsBaking => coroutine != null;

        [HideInInspector]
        public List<string> EnabledSettingNames = new List<string>();

        List<PropertyInfoDataWrapper> enabledProperties = new List<PropertyInfoDataWrapper>();

        public int CurrentExport
        {
            get
            {
                if (IsBaking)
                {
                    return runner.counter;
                }
                return 0;
            }
        }

        public int PermutationCount
        {
            get
            {
                int total = 1;

                foreach (var prop in EnabledProperties)
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

        public ReadOnlyCollection<PropertyInfoDataWrapper> EnabledProperties
        {
            get
            {
                enabledProperties.Clear();

                if (Server.propertyInfos != null)
                {
                    foreach (var prop in Server.propertyInfos)
                    {
                        if (IsEnabled(prop))
                        {
                            enabledProperties.Add(prop);
                        }
                    }
                }

                return enabledProperties.AsReadOnly();
            }
        }

        public MeshSyncServer Server
        {
            get { return server; }
            set
            {
                if (server != value)
                {
                    server = value;

                    EnabledSettingNames.Clear();

                    if (server != null)
                    {
                        foreach (var prop in server.propertyInfos)
                        {
                            EnabledSettingNames.Add(prop.name);
                        }
                    }
                }
            }
        }

        EditorCoroutine coroutine;

        PermutationRunner runner;

        public bool IsEnabled(PropertyInfoDataWrapper property)
        {
            return EnabledSettingNames.Contains(property.name);
        }

        private void OnEnable()
        {
            if (Server == null)
            {
                Server = FindObjectOfType<MeshSyncServer>();
            }
        }

        public void Export()
        {
            if (ExportMode == ExportModeSetting.RegenerateEverything)
            {
                if (!EditorUtility.DisplayDialog("Warning", $"This will delete all previously exported assets in the \"{SaveFile}\" folder. Are you sure?", "Yes", "No"))
                {
                    return;
                }
            }
            else if (ExportMode == ExportModeSetting.RegenerateOnlyExisting)
            {
                if (!EditorUtility.DisplayDialog("Warning", $"This will overwrite all previously exported assets in the \"{SaveFile}\" folder. Are you sure?", "Yes", "No"))
                {
                    return;
                }
            }

            runner = new PermutationRunner(this);

            AssetDatabase.StartAssetEditing();

            coroutine = EditorCoroutineUtility.StartCoroutine(runner.Next(), this);
        }

        public void StopExport()
        {
            EditorUtility.ClearProgressBar();

            if (coroutine != null)
            {
                EditorCoroutineUtility.StopCoroutine(coroutine);
                coroutine = null;
            }

            AssetDatabase.StopAssetEditing();
        }

        class WaitForMeshSync : CustomYieldInstruction
        {
            MeshSyncServer server;

            public WaitForMeshSync(MeshSyncServer server)
            {
                this.server = server;
            }

            public override bool keepWaiting => server.CurrentPropertiesState != MeshSyncServer.PropertiesState.Received;
        }

        class PermutationRunner
        {
            VariantExporter variantExporter;

            MeshSyncServer server => variantExporter.Server;

            enum SaveMode
            {
                DryRunWithoutSyncAndSave,
                SyncButDoNotSave,
                SaveAsPrefab
            }

            object[] currentSettings;

            public int counter;

            SaveMode saveMode = SaveMode.SaveAsPrefab;

            public ReadOnlyCollection<PropertyInfoDataWrapper> propertyInfos => variantExporter.EnabledProperties;

            string GetSavePath()
            {
                return Path.Combine("Assets", variantExporter.SaveFile);
            }

            public PermutationRunner(VariantExporter baker)
            {
                this.variantExporter = baker;

                currentSettings = new object[variantExporter.enabledProperties.Count];

                var outputFilePath = GetSavePath();

                // For now delete the folder if it exists, maybe we'd want to resume where we left off if there are files in there?
                if (Directory.Exists(outputFilePath))
                {
                    Directory.Delete(outputFilePath, true);

                    AssetDatabase.Refresh();
                }
            }

            public IEnumerator Next(int propIdx = -1)
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
                var outputFilePath = GetSavePath();

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
                            var asset = Instantiate(child.sharedMesh);
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

            IEnumerator ExecutePermutations(int propIdx)
            {
                PropertyInfoDataWrapper property = propertyInfos[propIdx];

                switch (property.type)
                {
                    case PropertyInfoData.Type.Int:
                        for (int i = (int)property.min; i <= property.max; i++)
                        {
                            currentSettings[propIdx] = i;

                            yield return Next(propIdx);
                        }
                        break;
                    default:
                        yield return Next(propIdx);
                        break;
                }
            }
        }
    }
}