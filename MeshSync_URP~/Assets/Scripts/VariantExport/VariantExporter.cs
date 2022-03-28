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

namespace Unity.MeshSync.VariantExport
{
    [ExecuteAlways]
    public class VariantExporter : MonoBehaviour
    {
        public enum ExportModeSetting
        {
            RegenerateEverything,
            RegenerateOnlyExisting,
        }

        EditorCoroutine coroutine;

        PermutationRunnerBase currentRunner;

        PermutationRunnerBase runner
        {
            get
            {
                if (currentRunner == null)
                {
                    currentRunner = new AllPermutationRunner(this);
                }

                return currentRunner;
            }
        }

        [SerializeField]
        private MeshSyncServer server;

        public string SaveFile;

        public ExportModeSetting ExportMode;

        public bool IsBaking => coroutine != null;

        [HideInInspector]
        public List<string> EnabledSettingNames = new List<string>();

        public List<PropertyInfoDataWrapper> enabledProperties = new List<PropertyInfoDataWrapper>();

        public int CurrentExport => runner.counter;

        public int PermutationCount => runner.PermutationCount;

        public string SavePath => Path.Combine("Assets", SaveFile);

        public ReadOnlyCollection<PropertyInfoDataWrapper> EnabledProperties
        {
            get
            {
                enabledProperties.Clear();

                if (Server?.propertyInfos != null)
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
            get => server;
            set => server = value;
        }

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

            AssetDatabase.StartAssetEditing();

            coroutine = EditorCoroutineUtility.StartCoroutine(runner.Start(), this);
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

            currentRunner = null;
        }
    }
}