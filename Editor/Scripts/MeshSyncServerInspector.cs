using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {
    [CustomEditor(typeof(MeshSyncServer))]
    internal class MeshSyncServerInspector : MeshSyncPlayerInspector   {
        
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

        public override void OnEnable() {
            base.OnEnable();
            m_serverAsset = target as MeshSyncServer;

        }

//----------------------------------------------------------------------------------------------------------------------
        public override void OnInspectorGUI()
        {
            Undo.RecordObject(m_serverAsset, "SceneCachePlayer Update");        
            SerializedObject so = serializedObject;

            EditorGUILayout.Space();
            DrawServerSettings(m_serverAsset, so);
            DrawPlayerSettings(m_serverAsset, so);
            DrawMaterialList(m_serverAsset);
            DrawTextureList(m_serverAsset);
            DrawAnimationTweak(m_serverAsset);
            DrawExportAssets(m_serverAsset);
            DrawPluginVersion();

            so.ApplyModifiedProperties();
            PrefabUtility.RecordPrefabInstancePropertyModifications(m_serverAsset);
            
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
                int serverPort = EditorGUILayout.IntField("Server Port:", (int) m_serverAsset.GetServerPort());
                m_serverAsset.SetServerPort((ushort) serverPort);
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
