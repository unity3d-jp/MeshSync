using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {
    [CustomEditor(typeof(MeshSyncServer))]
    internal class MeshSyncServerInspector : MeshSyncPlayerInspector   {
        
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

        public override void OnEnable() {
            base.OnEnable();
            m_meshSyncServer = target as MeshSyncServer;

        }

//----------------------------------------------------------------------------------------------------------------------
        public override void OnInspectorGUI()
        {
            Undo.RecordObject(m_meshSyncServer, "MeshSyncServer Update");        
            SerializedObject so = serializedObject;

            EditorGUILayout.Space();
            DrawServerSettings(m_meshSyncServer, so);
            DrawPlayerSettings(m_meshSyncServer, so);
            DrawMaterialList(m_meshSyncServer);
            DrawTextureList(m_meshSyncServer);
            DrawAnimationTweak(m_meshSyncServer);
            DrawExportAssets(m_meshSyncServer);
            DrawPluginVersion();

            so.ApplyModifiedProperties();
            PrefabUtility.RecordPrefabInstancePropertyModifications(m_meshSyncServer);
            
        }
//----------------------------------------------------------------------------------------------------------------------

        public void DrawServerSettings(MeshSyncServer t, SerializedObject so)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;

            bool isServerStarted = m_meshSyncServer.IsServerStarted();
            string serverStatus = isServerStarted ? "Server (Status: Started)" : "Server (Status: Stopped)";
            t.foldServerSettings= EditorGUILayout.Foldout(t.foldServerSettings, serverStatus, true, styleFold);
            if (t.foldServerSettings) {
                
                bool autoStart = EditorGUILayout.Toggle("Auto Start", m_meshSyncServer.IsAutoStart());
                m_meshSyncServer.SetAutoStartServer(autoStart);

                //Draw GUI that are disabled when autoStart is true
                EditorGUI.BeginDisabledGroup(autoStart);
                int serverPort = EditorGUILayout.IntField("Server Port:", (int) m_meshSyncServer.GetServerPort());
                m_meshSyncServer.SetServerPort((ushort) serverPort);
                GUILayout.BeginHorizontal();
                if (isServerStarted) {
                    if (GUILayout.Button("Stop", GUILayout.Width(110.0f))) {
                        m_meshSyncServer.StopServer();
                    }
                } else {
                    if (GUILayout.Button("Start", GUILayout.Width(110.0f))) {
                        m_meshSyncServer.StartServer();
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
        private MeshSyncServer m_meshSyncServer = null;        
    }
}
