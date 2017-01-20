using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    [CustomEditor(typeof(MeshSyncServer))]
    public class MeshSyncServerEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            var t = target as MeshSyncServer;

            EditorGUILayout.Space();
            if (GUILayout.Button("Generate Lightmap UV"))
            {
                t.GenerateLightmapUV();
            }
            if (GUILayout.Button("Export Meshes"))
            {
                t.ExportMeshes();
            }
        }
    }
}