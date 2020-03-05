using System;
using Unity.MeshSync;
using UnityEditor;
using UnityEngine;

namespace UnityEditor.MeshSync
{
    [CustomEditor(typeof(MeshSyncServer))]
    internal class MeshSyncServerEditor : MeshSyncPlayerEditor   {
        [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
        internal static void CreateMeshSyncServerMenu(MenuCommand menuCommand) {
            Transform t = CreateMeshSyncServer();
            if (t != null)
                Undo.RegisterCreatedObjectUndo(t.gameObject, "MeshSyncServer");
            Selection.activeTransform = t;
        }
        
//----------------------------------------------------------------------------------------------------------------------
        internal static Transform CreateMeshSyncServer() {
            GameObject go = new GameObject("MeshSyncServer");
            MeshSyncServer mss = go.AddComponent<MeshSyncServer>();
            Transform t = go.GetComponent<Transform>();
            mss.rootObject = t;
            return t;
        }

//----------------------------------------------------------------------------------------------------------------------

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
