﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System;

namespace Unity.MeshSync.Editor
{
    public static class MeshSyncServerInspectorUtils
    {
        static List<string> foldouts = new List<string>();

        public static void OpenDCCAsset(MeshSyncServer server)
        {
            BlenderLauncher.OpenBlendFile(server, server.m_DCCAsset);
        }

        public static void DrawSliderForProperties(List<PropertyInfoDataWrapper> props,
            Action<PropertyInfoDataWrapper> before = null,
            Action<PropertyInfoDataWrapper> after = null,
            Func<string, bool> allEnabled = null,
            Action<string, bool> allEnabledChanged = null)
        {
            var lastPathDrawn = string.Empty;
            bool expanded = false;

            foreach (var prop in props)
            {
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
                    case PropertyInfoData.Type.Int:
                        int max = (int)prop.max;

                        // Need to be careful with overflow here:
                        if (prop.max == int.MaxValue)
                        {
                            max = int.MaxValue - 1;
                        }

                        newValue = EditorGUILayout.IntSlider(prop.name, prop.GetValue<int>(), (int)prop.min, max);
                        break;

                    case PropertyInfoData.Type.Float:
                        newValue = EditorGUILayout.Slider(prop.name, prop.GetValue<float>(), prop.min, prop.max);
                        break;

                    case PropertyInfoData.Type.String:
                        EditorGUILayout.LabelField(prop.name, prop.GetValue<string>());
                        break;

                    case PropertyInfoData.Type.IntArray:
                        newValue = DrawArrayFieldsInt(prop, newValue);
                        break;

                    case PropertyInfoData.Type.FloatArray:
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
    }

    // Partial class for now to make merging code easier later.
    [CustomEditor(typeof(MeshSyncServerProperties))]
    class MeshSyncServerPropertiesInspector : UnityEditor.Editor
    {
        public static IDCCLauncher GetLauncherForAsset(UnityEngine.Object asset)
        {
            // TODO: Check asset path here and choose IDCCLauncher implementation for the given type.
            // var assetPath = AssetDatabase.GetAssetPath(asset).Replace("Assets/", string.Empty);
            return new BlenderLauncher();
        }

        public override void OnInspectorGUI()
        {
            var propertiesHolder = (MeshSyncServerProperties)target;

            var server = propertiesHolder.Server;

            if (server != null)
            {
                GUILayout.BeginHorizontal();

                server.m_DCCAsset = EditorGUILayout.ObjectField("Blend file:", server.m_DCCAsset, typeof(UnityEngine.Object), true);
                if (server.m_DCCAsset != null)
                {
                    if (GUILayout.Button("Open"))
                    {
                        MeshSyncServerInspectorUtils.OpenDCCAsset(server);
                    }
                }

                GUILayout.EndHorizontal();

                server.m_DCCInterop?.DrawDCCToolVersion(server);
            }

            MeshSyncServerInspectorUtils.DrawSliderForProperties(propertiesHolder.propertyInfos);
        }
    }
}
