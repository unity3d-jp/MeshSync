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

            Debug.Log(s);

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

        IEnumerator SavePrefab()
        {
            var outputFilePath = regenerator.SavePath;

            if (!Directory.Exists(outputFilePath))
            {
                Directory.CreateDirectory(outputFilePath);
            }

            // Make a copy, apply the prefab-specific save location and save any changed assets to that:
            var prefabSave = UnityEngine.Object.Instantiate(server.gameObject);
            var prefabServer = prefabSave.GetComponent<MeshSyncServer>();
            prefabServer.enabled = false;

            var prefabAssetStorePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_{counter}_assets");

            if (Directory.Exists(prefabAssetStorePath))
            {
                AssetDatabase.DeleteAsset(prefabAssetStorePath);
            }

            prefabServer.CopySettingsFrom(server);
            prefabServer.SetAssetsFolder(prefabAssetStorePath);

            prefabServer.ExportMeshes();

            // If there are no assets specific to this prefab, delete the empty folder:
            if (Directory.GetFiles(prefabAssetStorePath).Length == 0)
            {
                AssetDatabase.DeleteAsset(prefabAssetStorePath);
            }
            else
            {
                AssetDatabase.SaveAssets();
            }

            outputFilePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_{counter}.prefab");
            PrefabUtility.SaveAsPrefabAsset(prefabSave, outputFilePath);

            UnityEngine.Object.DestroyImmediate(prefabSave);

            yield break;
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

            var commonAssetStorePath = Path.Combine(outputFilePath, $"{regenerator.SaveFile}_assets");

            regenerator.Server.SetAssetsFolder(commonAssetStorePath);
            regenerator.Server.ExportMeshes();
            regenerator.Server.ExportMaterials();

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
