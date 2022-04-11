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
    public class Regenerator : MonoBehaviour
    {
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

        public MeshSyncServer Server;

        public List<string> Whitelist = new List<string>();
        public List<string> Blacklist = new List<string>();

        public string SaveFile;

        public bool AutoShuffle;

        public bool IsBaking => coroutine != null;

        [HideInInspector]
        public List<string> EnabledSettingNames = new List<string>();

        public List<PropertyInfoDataWrapper> enabledProperties = new List<PropertyInfoDataWrapper>();

        public int CurrentExport => runner.counter;

        public int VariantCount => runner.VariantCount;

        public int TotalPermutationCount => runner.TotalPermutationCount;

        public bool CheckForCancellation()
        {
            return runner.CheckForCancellation();
        }

        public string SavePath
        {
            get
            {
                if (SaveFile == null)
                {
                    return null;
                }

                return Path.Combine("Assets", SaveFile);
            }
        }

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
            try
            {
                lock (PropertyInfoDataWrapper.PropertyUpdateLock)
                {
                    for (int tries = 0; tries < 1000; tries++)
                    {
                        foreach (var prop in EnabledProperties)
                        {
                            switch (prop.type)
                            {
                                case PropertyInfoData.Type.Int:
                                    prop.NewValue = UnityEngine.Random.Range((int)prop.min, (int)prop.max + 1);
                                    break;

                                case PropertyInfoData.Type.Float:
                                    prop.NewValue = UnityEngine.Random.Range(prop.min, prop.max);
                                    break;

                                case PropertyInfoData.Type.FloatArray:
                                    {
                                        var array = new float[prop.arrayLength];
                                        for (int i = 0; i < array.Length; i++)
                                        {
                                            array[i] = UnityEngine.Random.Range(prop.min, prop.max);
                                        }

                                        prop.NewValue = array;
                                        break;
                                    }

                                case PropertyInfoData.Type.IntArray:
                                    {
                                        var array = new int[prop.arrayLength];
                                        for (int i = 0; i < array.Length; i++)
                                        {
                                            array[i] = UnityEngine.Random.Range((int)prop.min, (int)prop.max + 1);
                                        }

                                        prop.NewValue = array;
                                        break;
                                    }
                            }
                        }

                        // Try to ensure this variant is not already blocked or kept:
                        var props = SerializeCurrentVariant(true);
                        if (!Whitelist.Contains(props) && !Blacklist.Contains(props))
                        {
                            break;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.LogException(ex);
            }
        }

        public string SerializeCurrentVariant(bool useNewValues = false)
        {
            if (Server?.propertyInfos == null)
            {
                return null;
            }

            var sb = new StringBuilder();

            foreach (var prop in Server.propertyInfos)
            {
                if (!prop.CanBeModified)
                {
                    continue;
                }

                string serializedValue = $"#{prop.name}:";

                serializedValue += prop.GetSerializedValue(useNewValues);

                sb.Append(serializedValue);
            }

            return sb.ToString();
        }

        public void ApplySerializedProperties(string serializedPropString)
        {
            if (Server?.propertyInfos == null)
            {
                return;
            }

            lock (PropertyInfoDataWrapper.PropertyUpdateLock)
            {
                var propStrings = serializedPropString.Split('#');

                var properties = Server.propertyInfos;

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

                            // Force sync to make sure the sync state changes:
                            serverProp.IsDirty = true;

                            break;
                        }
                    }
                }
            }
        }

        public void Clear()
        {
            Whitelist.Clear();
            Blacklist.Clear();
        }

        public void PreviousKeptVariant()
        {
            MoveInList(Whitelist, -1);
        }

        public void NextKeptVariant()
        {
            MoveInList(Whitelist, +1);
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

                if (AutoShuffle)
                {
                    Shuffle();
                }
            }
        }

        public bool IsCurrentVariantBlocked => Blacklist.Contains(SerializeCurrentVariant());

        public bool IsCurrentVariantKept => Whitelist.Contains(SerializeCurrentVariant());

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

                if (AutoShuffle)
                {
                    Shuffle();
                }
            }
        }

        public void DoNotKeepVariant()
        {
            Whitelist.Remove(SerializeCurrentVariant());
        }

        public void UnblockVariant()
        {
            Blacklist.Remove(SerializeCurrentVariant());
        }

        public void PreviousBlockedVariant()
        {
            MoveInList(Blacklist, -1);
        }

        public void NextBlockedVariant()
        {
            MoveInList(Blacklist, +1);
        }

        void MoveInList(List<string> list, int move)
        {
            if (list.Count == 0)
            {
                return;
            }

            int newIndex = 0;
            var current = list.IndexOf(SerializeCurrentVariant());
            if (current != -1)
            {
                newIndex = (current + move) % list.Count;

                if (newIndex < 0)
                {
                    newIndex += list.Count;
                }
            }

            ApplySerializedProperties(list[newIndex]);
        }

        public void Regenerate()
        {
            if (SaveFile?.Length == 0)
            {
                EditorUtility.DisplayDialog("Warning", "Cannot export. SaveFile is not set.", "OK");
                return;
            }

            if (Directory.Exists(SavePath))
            {
                if (!EditorUtility.DisplayDialog("Warning", $"This will delete all previously exported prefabs in the \"{SaveFile}\" folder. Are you sure?", "Yes", "No"))
                {
                    return;
                }
            }

            AssetDatabase.StartAssetEditing();

            coroutine = EditorCoroutineUtility.StartCoroutine(runner.Start(), this);
        }

        public void StopExport()
        {
            if (coroutine != null)
            {
                EditorCoroutineUtility.StopCoroutine(coroutine);
                coroutine = null;
            }

            EditorUtility.ClearProgressBar();

            AssetDatabase.StopAssetEditing();

            currentRunner = null;
        }
    }
}