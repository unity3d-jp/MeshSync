using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor
{
    public static class MeshSyncServerInspectorUtils
    {
        public static void DrawSliderForProperty(PropertyInfoDataWrapper prop)
        {
            EditorGUI.BeginChangeCheck();

            object newValue = null;
            switch (prop.type)
            {
                case PropertyInfoData.Type.Int:
                    newValue = EditorGUILayout.IntSlider(prop.name, prop.GetValue<int>(), (int)prop.min, (int)prop.max);
                    break;

                case PropertyInfoData.Type.Float:
                    newValue = EditorGUILayout.Slider(prop.name, prop.GetValue<float>(), prop.min, prop.max);
                    break;

                case PropertyInfoData.Type.String:
                    EditorGUILayout.LabelField(prop.name, prop.GetValue<string>());
                    break;

                case PropertyInfoData.Type.IntArray:
                    {
                        // TODO: Use custom IntVector here maybe:
                        var a = prop.GetValue<int[]>();
                        if (prop.arrayLength == 3)
                        {
                            var v = new Vector3(a[0], a[1], a[2]);
                            newValue = EditorGUILayout.Vector3Field(prop.name, v);
                        }
                        break;
                    }
                case PropertyInfoData.Type.FloatArray:
                    {
                        var a = prop.GetValue<float[]>();
                        if (prop.arrayLength == 3)
                        {
                            var v = new Vector3(a[0], a[1], a[2]);
                            newValue = EditorGUILayout.Vector3Field(prop.name, v);
                        }
                        break;
                    }
            }

            if (EditorGUI.EndChangeCheck())
            {
                prop.NewValue = newValue;
            }
        }
    }

    // Partial class for now to make merging code easier later.
    partial class MeshSyncServerInspector
    {
        public static IDCCLauncher GetLauncherForAsset(GameObject asset)
        {
            // TODO: Check asset path here and choose IDCCLauncher implementation for the given type.
            // var assetPath = AssetDatabase.GetAssetPath(asset).Replace("Assets/", string.Empty);
            return new BlenderLauncher();
        }

        void DrawSliders(MeshSyncServer server)
        {
            var style = EditorStyles.foldout;
            style.fontStyle = FontStyle.Bold;
            server.foldBlenderSettings = EditorGUILayout.Foldout(server.foldBlenderSettings, "Blender settings", true, style);
            if (!server.foldBlenderSettings)
            {
                return;
            }

            server.m_DCCInterop?.DrawDCCToolVersion(server);

            var properties = server.propertyInfos;

            for (int i = 0; i < properties.Count; i++)
            {
                MeshSyncServerInspectorUtils.DrawSliderForProperty(properties[i]);
            }
        }
    }
}
