using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;

namespace Unity.MeshSync.Editor
{
    public static class MeshSyncServerInspectorUtils
    {
        static List<string> foldouts = new List<string>();

        public static void OpenDCCAsset(MeshSyncServer server)
        {
            var previousRunMode = server.m_DCCInterop != null ? server.m_DCCInterop.runMode : RunMode.GUI;

            var asset = server.m_DCCAsset;

            server.m_DCCInterop?.Cleanup();
            server.m_DCCInterop = GetLauncherForAsset(asset);

            if (server.m_DCCInterop != null)
            {
                server.m_DCCInterop.runMode = previousRunMode;
                server.m_DCCInterop.OpenDCCTool(asset);
            }
            else
            {
                var assetPath = AssetDatabase.GetAssetPath(asset).Replace("Assets/", string.Empty);
                var extension = Path.GetExtension(assetPath);
                Debug.LogError($"No DCC handler for {extension} files is implemented.");
            }
        }

        public static void DrawSliderForProperties(List<PropertyInfoDataWrapper> props,
            bool editableStrings,
            string[] parentNameFilters = null,
            Action<PropertyInfoDataWrapper> before = null,
            Action<PropertyInfoDataWrapper> after = null,
            Func<string, bool> allEnabled = null,
            Action<string, bool> allEnabledChanged = null)
        {
            var lastPathDrawn = string.Empty;
            bool expanded = false;

            foreach (var prop in props)
            {
                if (parentNameFilters != null && Array.IndexOf(parentNameFilters, prop.path) == -1)
                {
                    continue;
                }

                if (prop.path != lastPathDrawn)
                {
                    bool wasExpanded = foldouts.Contains(prop.path);
                    expanded = EditorGUILayout.Foldout(wasExpanded, prop.path);

                    if (wasExpanded != expanded)
                    {
                        if (wasExpanded)
                        {
                            foldouts.Remove(prop.path);
                        }
                        else
                        {
                            foldouts.Add(prop.path);
                        }
                    }

                    lastPathDrawn = prop.path;

                    if (expanded && allEnabled != null && allEnabledChanged != null)
                    {
                        var wasAllEnabled = allEnabled(lastPathDrawn);

                        bool newAllEnabled = EditorGUILayout.ToggleLeft($"All properties in {lastPathDrawn}", wasAllEnabled);
                        if (newAllEnabled != wasAllEnabled)
                        {
                            allEnabledChanged(lastPathDrawn, newAllEnabled);
                        }
                    }
                }

                if (!expanded)
                {
                    continue;
                }

                before?.Invoke(prop);

                EditorGUI.BeginChangeCheck();

                object newValue = null;
                switch (prop.type)
                {
                    case PropertyInfoDataType.Int:
                        int max = (int)prop.max;

                        // Need to be careful with overflow here:
                        if (prop.max == int.MaxValue)
                        {
                            max = int.MaxValue - 1;
                        }

                        newValue = EditorGUILayout.IntSlider(prop.name, prop.GetValue<int>(), (int)prop.min, max);
                        break;

                    case PropertyInfoDataType.Float:
                        newValue = EditorGUILayout.Slider(prop.name, prop.GetValue<float>(), prop.min, prop.max);
                        break;

                    case PropertyInfoDataType.String:
                        if (editableStrings)
                        {
                            newValue = EditorGUILayout.TextField(prop.name, prop.GetValue<string>());
                        }
                        else
                        {
                            EditorGUILayout.LabelField(prop.name, prop.GetValue<string>());
                        }
                        break;

                    case PropertyInfoDataType.IntArray:
                        newValue = DrawArrayFieldsInt(prop, newValue);
                        break;

                    case PropertyInfoDataType.FloatArray:
                        newValue = DrawArrayFieldsFloat(prop, newValue);
                        break;

                    default:
                        break;
                }

                if (EditorGUI.EndChangeCheck())
                {
                    lock (PropertyInfoDataWrapper.PropertyUpdateLock)
                    {
                        prop.NewValue = newValue;
                    }
                }

                after?.Invoke(prop);
            }
        }

        private static object DrawArrayFieldsFloat(PropertyInfoDataWrapper prop, object newValue)
        {
            var a = prop.GetValue<float[]>();
            switch (a.Length)
            {
                case 2:
                    {
                        newValue = EditorGUILayout.Vector2Field(prop.name, new Vector2(a[0], a[1]));
                        break;
                    }
                case 3:
                    {
                        newValue = EditorGUILayout.Vector3Field(prop.name, new Vector3(a[0], a[1], a[2]));
                        break;
                    }
                case 4:
                    {
                        newValue = EditorGUILayout.Vector4Field(prop.name, new Vector4(a[0], a[1], a[2], a[3]));
                        break;
                    }
                default:
                    {
                        EditorGUILayout.BeginVertical();
                        for (int i = 0; i < prop.arrayLength; i++)
                        {
                            EditorGUI.BeginChangeCheck();

                            var newValueInArray = EditorGUILayout.FloatField($"{prop.name}[{i}]", a[i]);

                            if (EditorGUI.EndChangeCheck())
                            {
                                var newValueAsArray = new float[prop.arrayLength];
                                for (int j = 0; j < prop.arrayLength; j++)
                                {
                                    if (j == i)
                                    {
                                        newValueAsArray[j] = newValueInArray;
                                    }
                                    else
                                    {
                                        newValueAsArray[j] = a[j];
                                    }
                                }

                                newValue = newValueAsArray;
                            }
                        }
                        EditorGUILayout.EndVertical();
                        break;
                    }
            }

            return newValue;
        }

        private static object DrawArrayFieldsInt(PropertyInfoDataWrapper prop, object newValue)
        {
            var a = prop.GetValue<int[]>();
            switch (a.Length)
            {
                case 2:
                    {
                        newValue = EditorGUILayout.Vector2IntField(prop.name, new Vector2Int(a[0], a[1]));
                        break;
                    }
                case 3:
                    {
                        newValue = EditorGUILayout.Vector3IntField(prop.name, new Vector3Int(a[0], a[1], a[2]));
                        break;
                    }
                case 4:
                    {
                        newValue = EditorGUILayout.Vector4Field(prop.name, new Vector4(a[0], a[1], a[2], a[3]));
                        break;
                    }
                default:
                    {
                        EditorGUILayout.BeginVertical();
                        for (int i = 0; i < prop.arrayLength; i++)
                        {
                            EditorGUI.BeginChangeCheck();

                            var newValueInArray = EditorGUILayout.IntField($"{prop.name}[{i}]", a[i]);

                            if (EditorGUI.EndChangeCheck())
                            {
                                var newValueAsArray = new int[prop.arrayLength];
                                for (int j = 0; j < prop.arrayLength; j++)
                                {
                                    if (j == i)
                                    {
                                        newValueAsArray[j] = newValueInArray;
                                    }
                                    else
                                    {
                                        newValueAsArray[j] = a[j];
                                    }
                                }

                                newValue = newValueAsArray;
                            }
                        }
                        EditorGUILayout.EndVertical();
                        break;
                    }
            }

            return newValue;
        }
        internal static IDCCLauncher GetLauncherForAsset(UnityEngine.Object asset)
        {
            var assetPath = AssetDatabase.GetAssetPath(asset).Replace("Assets/", string.Empty);

            if (Path.GetExtension(assetPath) == ".blend")
            {
                return new BlenderLauncher();
            }

            // TODO: Implement and return launchers for other DCC file types here:

            return null;
        }
    }
}
