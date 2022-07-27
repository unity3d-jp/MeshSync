using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;
using System.Runtime.CompilerServices;

[assembly: InternalsVisibleTo("Unity.Utilities.VariantExport")]

namespace Unity.MeshSync.Editor {
    internal static class MeshSyncServerInspectorUtils {
        static HashSet<string> foldouts = new HashSet<string>();
        
        const int MAX_SLIDER_RANGE = 1000;


        public static void DrawSliderForProperties(List<PropertyInfoDataWrapper> props,
            bool editableStrings,
            string[] parentNameFilters = null,
            Action<PropertyInfoDataWrapper> before = null,
            Action<PropertyInfoDataWrapper> after = null,
            Func<string, bool> allEnabled = null,
            Action<string, bool> allEnabledChanged = null) {
            var lastPathDrawn = string.Empty;
            bool expanded = false;

            foreach (var prop in props) {
                if (parentNameFilters != null && Array.IndexOf(parentNameFilters, prop.path) == -1) {
                    continue;
                }

                if (prop.path != lastPathDrawn) {
                    bool wasExpanded = foldouts.Contains(prop.path);
                    expanded = EditorGUILayout.Foldout(wasExpanded, prop.path);

                    if (wasExpanded != expanded) {
                        if (wasExpanded) {
                            foldouts.Remove(prop.path);
                        }
                        else {
                            foldouts.Add(prop.path);
                        }
                    }

                    lastPathDrawn = prop.path;

                    if (expanded && allEnabled != null && allEnabledChanged != null) {
                        var wasAllEnabled = allEnabled(lastPathDrawn);

                        bool newAllEnabled = EditorGUILayout.ToggleLeft($"All properties in {lastPathDrawn}", wasAllEnabled);
                        if (newAllEnabled != wasAllEnabled) {
                            allEnabledChanged(lastPathDrawn, newAllEnabled);
                        }
                    }
                }

                if (!expanded) {
                    continue;
                }

                before?.Invoke(prop);

                EditorGUI.BeginChangeCheck();

                object newValue = null;
                switch (prop.type) {
                    case PropertyInfoDataType.Int:
                        // Need to be careful with overflow here:
                        int max = Mathf.Min((int)prop.max, int.MaxValue - 1);

                        if (Mathf.Abs(max - prop.min) < MAX_SLIDER_RANGE) {
                            newValue = EditorGUILayout.IntSlider(prop.name, prop.GetValue<int>(), (int)prop.min, max);
                        }
                        else {
                            newValue = EditorGUILayout.IntField(prop.name, prop.GetValue<int>());
                        }

                        break;

                    case PropertyInfoDataType.Float:
                        if (Mathf.Abs(prop.max - prop.min) < MAX_SLIDER_RANGE) {
                            newValue = EditorGUILayout.Slider(prop.name, prop.GetValue<float>(), prop.min, prop.max);
                        }
                        else {
                            newValue = EditorGUILayout.FloatField(prop.name, prop.GetValue<float>());
                        }

                        break;

                    case PropertyInfoDataType.String:
                        if (editableStrings) {
                            newValue = EditorGUILayout.TextField(prop.name, prop.GetValue<string>());
                        }
                        else {
                            EditorGUILayout.LabelField(prop.name, prop.GetValue<string>());
                        }
                        break;

                    case PropertyInfoDataType.IntArray:
                        newValue = DrawArrayFieldsInt(prop);
                        break;

                    case PropertyInfoDataType.FloatArray:
                        newValue = DrawArrayFieldsFloat(prop);
                        break;

                    default:
                        break;
                }

                if (EditorGUI.EndChangeCheck()) {
                    lock (PropertyInfoDataWrapper.PropertyUpdateLock) {
                        prop.NewValue = newValue;
                    }
                }

                after?.Invoke(prop);
            }
        }

        static object DrawArrayFields<T>(PropertyInfoDataWrapper prop,
            Func<string, T[], object> draw2Dimensional,
            Func<string, T[], object> draw3Dimensional,
            Func<string, T[], object> draw4Dimensional,
            Func<string, T, T> fieldDrawMethodOther) {
            object newValue = null;
            var a = prop.GetValue<T[]>();

            switch (a.Length) {
                case 2: {
                        newValue = draw2Dimensional(prop.name, a);
                        break;
                    }
                case 3: {
                        newValue = draw3Dimensional(prop.name, a);
                        break;
                    }
                case 4: {
                        newValue = draw4Dimensional(prop.name, a);
                        break;
                    }
                default: {
                        EditorGUILayout.BeginVertical();

                        for (int i = 0; i < prop.arrayLength; i++) {
                            EditorGUI.BeginChangeCheck();

                            T newValueInArray = fieldDrawMethodOther($"{prop.name}[{i}]", a[i]);

                            if (EditorGUI.EndChangeCheck()) {
                                var newValueAsArray = new T[prop.arrayLength];
                                for (int j = 0; j < prop.arrayLength; j++) {
                                    if (j == i) {
                                        newValueAsArray[j] = newValueInArray;
                                    }
                                    else {
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

        private static object DrawArrayFieldsFloat(PropertyInfoDataWrapper prop) {
            return DrawArrayFields(prop,
                (string name, float[] val) => EditorGUILayout.Vector2Field(name, new Vector2(val[0], val[1])),
                (name, val) => EditorGUILayout.Vector3Field(name, new Vector3(val[0], val[1], val[2])),
                (name, val) => EditorGUILayout.Vector4Field(name, new Vector4(val[0], val[1], val[2], val[3])),
                (name, val) => EditorGUILayout.FloatField(name, val));
        }

        private static object DrawArrayFieldsInt(PropertyInfoDataWrapper prop) {
            return DrawArrayFields(prop,
                (name, val) => EditorGUILayout.Vector2IntField(name, new Vector2Int(val[0], val[1])),
                (name, val) => EditorGUILayout.Vector3IntField(name, new Vector3Int(val[0], val[1], val[2])),
                (name, val) => EditorGUILayout.Vector4Field(name, new Vector4(val[0], val[1], val[2], val[3])),
                (string name, int val) => EditorGUILayout.IntField(name, val));
        }

        public static void OpenDCCAsset(MeshSyncServer server) {
            var previousRunMode = server.m_DCCInterop != null ? server.m_DCCInterop.runMode : RunMode.GUI;

            var asset = server.DCCAsset;

            server.m_DCCInterop?.Dispose();
            server.m_DCCInterop = GetLauncherForAsset(asset);

            if (server.m_DCCInterop != null) {
                server.m_DCCInterop.runMode = previousRunMode;
                server.m_DCCInterop.OpenDCCTool(asset);
            }
            else {
                var assetPath = AssetDatabase.GetAssetPath(asset);
                var extension = Path.GetExtension(assetPath);
                EditorUtility.DisplayDialog("Error", $"No DCC handler for {extension} files is implemented.", "OK");
            }
        }

        internal static IDCCLauncher GetLauncherForAsset(UnityEngine.Object asset) {
            if (asset == null) {
                return null;
            }

            var assetPath = AssetDatabase.GetAssetPath(asset);

            if (Path.GetExtension(assetPath) == BlenderLauncher.FileFormat) {
                return new BlenderLauncher();
            }

            // TODO: Implement and return launchers for other DCC file types here:

            return null;
        }
    }
}
