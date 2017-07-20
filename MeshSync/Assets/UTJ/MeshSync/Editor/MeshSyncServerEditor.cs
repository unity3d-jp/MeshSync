using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ.MeshSync
{
    [CustomEditor(typeof(MeshSyncServer))]
    public class MeshSyncServerEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            var t = target as MeshSyncServer;

            GUILayout.Label("Material List", EditorStyles.boldLabel);
            DrawMaterialList(t);

            EditorGUILayout.Space();

            if (GUILayout.Button("Open Material Window"))
            {
                MaterialWindow.Open(t);
            }

            EditorGUILayout.Space();

            if (GUILayout.Button("Generate Lightmap UV"))
            {
                t.GenerateLightmapUV();
            }

            EditorGUILayout.Space();

            if (GUILayout.Button("Export Meshes"))
            {
                t.ExportMeshes();
            }
        }

        public static void DrawMaterialList(MeshSyncServer t)
        {
            foreach (var md in t.materialData)
            {
                if(md.id < 0) { continue; }

                var rect = EditorGUILayout.BeginHorizontal();
                EditorGUI.DrawRect(new Rect(rect.x, rect.y, 16, 16), md.color);
                EditorGUILayout.LabelField("", GUILayout.Width(16));
                EditorGUILayout.LabelField(md.name, GUILayout.Width(80));
                {
                    var tmp = EditorGUILayout.ObjectField(md.material, typeof(Material), true) as Material;
                    if (tmp != md.material)
                    {
                        Undo.RecordObject(t, "MeshSyncServer");
                        md.material = tmp;
                        t.ReassignMaterials();
                        t.ForceRepaint();
                    }
                }

                EditorGUILayout.EndHorizontal();
            }
        }
    }
}