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
            MeshSyncServer mss = CreateMeshSyncServer(true);
            if (mss != null)
                Undo.RegisterCreatedObjectUndo(mss.gameObject, "MeshSyncServer");
            Selection.activeTransform = mss.transform;
        }
        
//----------------------------------------------------------------------------------------------------------------------
        internal static MeshSyncServer CreateMeshSyncServer(bool autoStart) {
            GameObject go = new GameObject("MeshSyncServer");
            MeshSyncServer mss = go.AddComponent<MeshSyncServer>();
            Transform t = go.GetComponent<Transform>();
            mss.SetAutoStartServer(autoStart);
            mss.rootObject = t;
            return mss;
        }

//----------------------------------------------------------------------------------------------------------------------

        public override void OnEnable() {
            base.OnEnable();
            m_serverAsset = target as MeshSyncServer;

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

            bool isServerStarted = m_serverAsset.IsServerStarted();
            string serverStatus = isServerStarted ? "Server (Status: Started)" : "Server (Status: Stopped)";
            t.foldServerSettings= EditorGUILayout.Foldout(t.foldServerSettings, serverStatus, true, styleFold);
            if (t.foldServerSettings) {
                
                bool autoStart = EditorGUILayout.Toggle("Auto Start", m_serverAsset.IsAutoStart());
                m_serverAsset.SetAutoStartServer(autoStart);

                //Draw GUI that are disabled when autoStart is true
                EditorGUI.BeginDisabledGroup(autoStart);
                m_serverAsset.SetServerPort(EditorGUILayout.IntField("Server Port:", m_serverAsset.GetServerPort()));
                GUILayout.BeginHorizontal();
                if (isServerStarted) {
                    if (GUILayout.Button("Stop", GUILayout.Width(110.0f))) {
                        m_serverAsset.StopServer();
                    }
                } else {
                    if (GUILayout.Button("Start", GUILayout.Width(110.0f))) {
                        m_serverAsset.StartServer();
                    }
 
                }
                GUILayout.EndHorizontal();
                EditorGUI.EndDisabledGroup();
                
                EditorGUILayout.PropertyField(so.FindProperty("m_assetDir"));
                EditorGUILayout.PropertyField(so.FindProperty("m_rootObject"));
                EditorGUILayout.Space();
            }
        }

//----------------------------------------------------------------------------------------------------------------------                
        private MeshSyncServer m_serverAsset = null;        
    }
}
