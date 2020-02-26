using System;
using Unity.MeshSync;
using UnityEditor;
using UnityEngine;

namespace UnityEditor.MeshSync
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
            var t = target as MeshSyncServer;

            EditorGUILayout.Space();
            DrawServerSettings(t, so);
            DrawPlayerSettings(t, so);
            DrawMaterialList(t);
            DrawTextureList(t);
            DrawAnimationTweak(t);
            DrawExportAssets(t);
            DrawPluginVersion();

            so.ApplyModifiedProperties();
        }

        public void DrawServerSettings(MeshSyncServer t, SerializedObject so)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;

            t.foldServerSettings= EditorGUILayout.Foldout(t.foldServerSettings, "Server", true, styleFold);
            if (t.foldServerSettings)
            {
                EditorGUILayout.PropertyField(so.FindProperty("m_serverPort"));
                EditorGUILayout.PropertyField(so.FindProperty("m_assetDir"));
                EditorGUILayout.PropertyField(so.FindProperty("m_rootObject"));
                EditorGUILayout.Space();
            }
        }
    }
}
