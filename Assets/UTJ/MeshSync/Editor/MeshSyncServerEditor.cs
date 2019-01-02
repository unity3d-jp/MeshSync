using UnityEditor;
using UnityEngine;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
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

            //GUILayout.Label("Texture List", EditorStyles.boldLabel);
            //DrawTextureList(t);

            EditorGUILayout.Space();

            if (GUILayout.Button("Open Material Window"))
                MaterialWindow.Open(t);
            EditorGUILayout.Space();

            //if (GUILayout.Button("Generate Lightmap UV"))
            //    t.GenerateLightmapUV();
            //EditorGUILayout.Space();

            if (GUILayout.Button("Export Meshes"))
                t.ExportMeshes();
            EditorGUILayout.Space();

            if (GUILayout.Button("Export Materials"))
                t.ExportMaterials();
            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncServer.version);
        }

        public static void DrawMaterialList(MeshSyncServer t)
        {
            foreach (var md in t.materialData)
            {
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

        public static void DrawTextureList(MeshSyncServer t)
        {

        }
    }
}
