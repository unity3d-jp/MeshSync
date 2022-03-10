using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor
{
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
                var prop = properties[i];
                EditorGUI.BeginChangeCheck();

                object newValue = null;
                switch (prop.type)
                {
                    case PropertyInfoData.Type.Int:
                        newValue = (int)EditorGUILayout.Slider(prop.name, prop.GetValue<int>(), prop.min, prop.max);
                        break;

                    case PropertyInfoData.Type.Float:
                        newValue = EditorGUILayout.Slider(prop.name, prop.GetValue<float>(), prop.min, prop.max);
                        break;

                    case PropertyInfoData.Type.IntArray:
                        {
                            // TODO: Use custom IntVector here maybe:
                            var a = prop.GetValue<int[]>();
                            var v = new Vector3(a[0], a[1], a[2]);
                            newValue = EditorGUILayout.Vector3Field(prop.name, v);
                            break;
                        }
                    case PropertyInfoData.Type.FloatArray:
                        {
                            var a = prop.GetValue<float[]>();
                            var v = new Vector3(a[0], a[1], a[2]);
                            newValue = EditorGUILayout.Vector3Field(prop.name, v);
                            break;
                        }
                }

                if (EditorGUI.EndChangeCheck())
                {
                    switch (prop.type)
                    {
                        case PropertyInfoData.Type.IntArray:
                            {
                                // TODO: Use custom IntVector here maybe:
                                var newValueAsArray = new int[prop.arrayLength];
                                Vector3 newValueAsVector = (Vector3)newValue;

                                newValueAsArray[0] = (int)newValueAsVector.x;
                                newValueAsArray[1] = (int)newValueAsVector.y;
                                newValueAsArray[2] = (int)newValueAsVector.z;

                                newValue = newValueAsArray;
                                break;
                            }
                        case PropertyInfoData.Type.FloatArray:
                            {
                                var newValueAsArray = new float[prop.arrayLength];
                                Vector3 newValueAsVector = (Vector3)newValue;

                                newValueAsArray[0] = newValueAsVector.x;
                                newValueAsArray[1] = newValueAsVector.y;
                                newValueAsArray[2] = newValueAsVector.z;

                                newValue = newValueAsArray;
                                break;
                            }
                    }

                    prop.NewValue = newValue;
                }
            }
        }
    }
}
