using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(VariantExporter))]
    internal class VariantExporterEditor : UnityEditor.Editor
    {
        public override void OnInspectorGUI()
        {
            //base.OnInspectorGUI();

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
        }
    }
}
