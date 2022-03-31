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

        SelectedPermutationRunner currentRunner;

        SelectedPermutationRunner runner
        {
            get
            {
                if (currentRunner == null)
                {
                    //currentRunner = new AllPermutationRunner(this);
                    currentRunner = new SelectedPermutationRunner(this);
                }

                return currentRunner;
            }
        }

        [SerializeField]
        private MeshSyncServer server;

        public List<string> Whitelist = new List<string>();
        public List<string> Blacklist = new List<string>();

        public string SaveFile;

        //public ExportModeSetting ExportMode;

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

        public void Shuffle()
        {
            foreach (var prop in EnabledProperties)
            {
                switch (prop.type)
                {
                    case PropertyInfoData.Type.Int:
                        prop.NewValue = UnityEngine.Random.Range((int)prop.min, (int)prop.max + 1);
                        break;
                }
            }
        }

        string SerializeCurrentVariant()
        {
            if (Server?.propertyInfos == null)
            {
                return null;
            }

            var sb = new StringBuilder();

            foreach (var prop in Server.propertyInfos)
            {
                // Ignore strings, they don't change:
                if (prop.type == PropertyInfoData.Type.String)
                {
                    continue;
                }

                string serializedValue = $"#{prop.name}:";

                switch (prop.type)
                {
                    case PropertyInfoData.Type.Int:
                        serializedValue += prop.GetSerializedValue();
                        break;
                }

                sb.Append(serializedValue);
            }

            return sb.ToString();
        }

        public void DeserializeProps(string serializedPropString)
        {
            if (Server?.propertyInfos == null)
            {
                return;
            }

            var propStrings = serializedPropString.Split('#');

            var properties = server.propertyInfos;

            foreach (var propString in propStrings)
            {
                if (propString.Length == 0)
                {
                    continue;
                }

                var propName = propString.Substring(0, propString.IndexOf(":"));

                foreach (var serverProp in properties)
                {
                    if (serverProp.name == propName)
                    {
                        serverProp.SetSerializedValue(propString.Substring(propString.IndexOf(":") + 1));

                        break;
                    }
                }
            }
        }

        public void PreviousKeptVariant()
        {
            if (Whitelist.Count == 0)
            {
                return;
            }

            var current = Whitelist.IndexOf(SerializeCurrentVariant());
            var previous = current - 1;
            if (previous < 0)
            {
                previous += Whitelist.Count;
            }

            DeserializeProps(Whitelist[previous]);
        }

        public void NextKeptVariant()
        {
            if (Whitelist.Count == 0)
            {
                return;
            }

            var current = Whitelist.IndexOf(SerializeCurrentVariant());
            var next = (current + 1) % Whitelist.Count;

            DeserializeProps(Whitelist[next]);
        }

        public void KeepVariant()
        {
            var serialisedProps = SerializeCurrentVariant();

            if (serialisedProps != null)
            {
                Blacklist.Remove(serialisedProps);

                if (!Whitelist.Contains(serialisedProps))
                {
                    Whitelist.Add(serialisedProps);
                }
            }
        }

        public bool IsCurrentVariantBlocked
        {
            get
            {
                return Blacklist.Contains(SerializeCurrentVariant());
            }
        }

        public void BlockVariant()
        {
            var serialisedProps = SerializeCurrentVariant();

            if (serialisedProps != null)
            {
                Whitelist.Remove(serialisedProps);

                if (!Blacklist.Contains(serialisedProps))
                {
                    Blacklist.Add(serialisedProps);
                }
            }
        }

        public void UnblockVariant()
        {
            Blacklist.Remove(SerializeCurrentVariant());
        }

        public void PreviousBlockedVariant()
        {
            if (Blacklist.Count == 0)
            {
                return;
            }

            var current = Blacklist.IndexOf(SerializeCurrentVariant());
            if(current == -1)
            {
                current = 0;
            }

            var previous = current - 1;
            if (previous < 0)
            {
                previous += Blacklist.Count;
            }

            DeserializeProps(Blacklist[previous]);
        }

        public void NextBlockedVariant()
        {
            if (Blacklist.Count == 0)
            {
                return;
            }

            var current = Blacklist.IndexOf(SerializeCurrentVariant());
            var next = (current + 1) % Blacklist.Count;

            DeserializeProps(Blacklist[next]);
        }

        public void Export()
        {
            //if (ExportMode == ExportModeSetting.RegenerateEverything)
            //{
            //    if (!EditorUtility.DisplayDialog("Warning", $"This will delete all previously exported assets in the \"{SaveFile}\" folder. Are you sure?", "Yes", "No"))
            //    {
            //        return;
            //    }
            //}
            //else if (ExportMode == ExportModeSetting.RegenerateOnlyExisting)
            //{
            //    if (!EditorUtility.DisplayDialog("Warning", $"This will overwrite all previously exported assets in the \"{SaveFile}\" folder. Are you sure?", "Yes", "No"))
            //    {
            //        return;
            //    }
            //}

            if (!EditorUtility.DisplayDialog("Warning", $"This will delete all previously exported prefabs in the \"{SaveFile}\" folder. Are you sure?", "Yes", "No"))
            {
                return;
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