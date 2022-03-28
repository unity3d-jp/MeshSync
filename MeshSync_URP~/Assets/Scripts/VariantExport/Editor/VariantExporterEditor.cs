using System.Collections.Generic;
using System.IO;
using Unity.MeshSync.VariantExport;
using UnityEditor;
using UnityEngine;


namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(VariantExporter))]
    internal class VariantExporterEditor : UnityEditor.Editor
    {
        List<GameObject> variants = new List<GameObject>();

        public override void OnInspectorGUI()
        {
            var exporter = (VariantExporter)target;

            exporter.Server = (MeshSyncServer)EditorGUILayout.ObjectField("Server", exporter.Server, typeof(MeshSyncServer), true);
            exporter.SaveFile = EditorGUILayout.TextField("Save File", exporter.SaveFile);

            if (exporter.Server)
            {
                EditorGUILayout.LabelField("Properties to permutate:");

                foreach (var prop in exporter.Server.propertyInfos)
                {
                    if (prop.type != PropertyInfoData.Type.Int)
                    {
                        continue;
                    }

                    bool enabled = exporter.IsEnabled(prop);
                    bool newEnabled = EditorGUILayout.Toggle(prop.name, enabled);

                    if (newEnabled != enabled)
                    {
                        if (newEnabled)
                        {
                            exporter.EnabledSettingNames.Add(prop.name);
                        }
                        else
                        {
                            exporter.EnabledSettingNames.Remove(prop.name);
                        }
                    }
                }
            }

            if (!exporter.IsBaking)
            {
                exporter.ExportMode = (VariantExporter.ExportModeSetting)EditorGUILayout.EnumPopup(exporter.ExportMode);

                if (GUILayout.Button("Export!"))
                {
                    exporter.Export();
                }
            }
            else
            {
                if (GUILayout.Button("Stop!"))
                {
                    exporter.StopExport();
                }
            }

            EditorGUILayout.LabelField("Number of permutations", exporter.PermutationCount.ToString());

            if (Directory.Exists(exporter.SavePath))
            {
                if (GUILayout.Button("Show variants"))
                {
                    HideVariants();

                    Vector3 position = new Vector3(10, 0, 0);

                    foreach (var prefab in Directory.EnumerateFiles(exporter.SavePath, "*.prefab"))
                    {
                        var instance = (GameObject)PrefabUtility.InstantiatePrefab(AssetDatabase.LoadAssetAtPath<GameObject>(prefab));
                        var renderers = instance.GetComponentsInChildren<Renderer>(true);

                        float offset = 0;

                        foreach (var renderer in renderers)
                        {
                            offset = Mathf.Max(offset, renderer.bounds.size.x / 2);
                        }

                        offset += 1;

                        position.x += offset;

                        instance.transform.position = position;
                        variants.Add(instance);
                    }
                }

                if (variants.Count > 0 && GUILayout.Button("Hide variants"))
                {
                    HideVariants();
                }
            }
        }

        void HideVariants()
        {
            foreach (var variant in variants)
            {
                DestroyImmediate(variant);
            }
            variants.Clear();
        }
    }
}
