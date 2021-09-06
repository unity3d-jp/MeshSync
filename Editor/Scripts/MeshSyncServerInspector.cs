using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
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

        EditorGUILayout.Space();
        DrawServerSettings(m_meshSyncServer);
        DrawPlayerSettings(m_meshSyncServer);
        DrawMaterialList(m_meshSyncServer);
        DrawAnimationTweak(m_meshSyncServer);
        DrawExportAssets(m_meshSyncServer);
        DrawPluginVersion();

        PrefabUtility.RecordPrefabInstancePropertyModifications(m_meshSyncServer);
        
    }
//----------------------------------------------------------------------------------------------------------------------

    public void DrawServerSettings(MeshSyncServer t)
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
            
            string selectedFolder = EditorGUIDrawerUtility.DrawFolderSelectorGUI("Asset Dir", "Asset Dir", 
                t.GetAssetsFolder(), null);
            t.SetAssetsFolder(AssetUtility.NormalizeAssetPath(selectedFolder));
            
            Transform rootObject = (Transform) EditorGUILayout.ObjectField("Root Object", t.GetRootObject(), 
                typeof(Transform), allowSceneObjects: true);                
            t.SetRootObject(rootObject);
            
            EditorGUILayout.Space();
        }
    }

//----------------------------------------------------------------------------------------------------------------------                
    private MeshSyncServer m_meshSyncServer = null;        
}

} //end namespace
