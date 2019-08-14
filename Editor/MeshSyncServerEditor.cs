using System;
using UnityEditor;
using UnityEngine;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
{
    [CustomEditor(typeof(MeshSyncServer))]
    public class MeshSyncServerEditor : MeshSyncPlayerEditor
    {
        [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
        public static void CreateMeshSyncServerMenu(MenuCommand menuCommand)
        {
            var go = CreateMeshSyncServer();
            if (go != null)
                Undo.RegisterCreatedObjectUndo(go, "MeshSyncServer");
        }

        public static GameObject CreateMeshSyncServer()
        {
            var go = new GameObject();
            go.name = "MeshSyncServer";
            var mss = go.AddComponent<MeshSyncServer>();
            mss.rootObject = go.GetComponent<Transform>();
            return go;
        }


        public override void OnInspectorGUI()
        {
            var so = serializedObject;
            var t = target as MeshSyncPlayer;

            // server param
            EditorGUILayout.LabelField("Server", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_serverPort"));
            EditorGUILayout.PropertyField(so.FindProperty("m_assetDir"));
            EditorGUILayout.PropertyField(so.FindProperty("m_rootObject"));
            EditorGUILayout.Space();

            DrawPlayerSettings(t, so);
            DrawMaterialList(t);
            DrawTextureList(t);

            if (GUILayout.Button("Export Meshes", GUILayout.Width(160.0f)))
                t.ExportMeshes();
            EditorGUILayout.Space();

            if (GUILayout.Button("Export Materials", GUILayout.Width(160.0f)))
                t.ExportMaterials();
            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncPlayer.pluginVersion);
            so.ApplyModifiedProperties();
        }
    }
}
