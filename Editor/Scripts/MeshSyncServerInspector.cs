using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {
[CustomEditor(typeof(MeshSyncServer))]
internal class MeshSyncServerInspector : BaseMeshSyncInspector   {
    

//----------------------------------------------------------------------------------------------------------------------

    public void OnEnable() {
        m_meshSyncServer = target as MeshSyncServer;
    }

//----------------------------------------------------------------------------------------------------------------------
    public override void OnInspectorGUI()
    {
        Undo.RecordObject(m_meshSyncServer, "MeshSyncServer Update");        

        EditorGUILayout.Space();
        DrawServerSettings(m_meshSyncServer);
        DrawAssetSyncSettings(m_meshSyncServer);
        DrawImportSettings(m_meshSyncServer);
        DrawMiscSettings(m_meshSyncServer);
        DrawDefaultMaterialList(m_meshSyncServer);
        DrawAnimationTweak(m_meshSyncServer);
        DrawExportAssets(m_meshSyncServer);
        DrawInstanceSettings(m_meshSyncServer);
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

            string prevFolder = t.GetAssetsFolder(); 
            string selectedFolder = AssetEditorUtility.NormalizePath(
                EditorGUIDrawerUtility.DrawFolderSelectorGUI("Asset Dir", "Asset Dir", prevFolder, null)
            );
            if (selectedFolder != prevFolder) {
                if (string.IsNullOrEmpty(selectedFolder) || !AssetEditorUtility.IsPathNormalized(selectedFolder)) {
                    Debug.LogError($"[MeshSync] {selectedFolder} is not under Assets. Ignoring.");  
                } else {
                    t.SetAssetsFolder(selectedFolder);
                }
                
            }
            
            Transform rootObject = (Transform) EditorGUILayout.ObjectField("Root Object", t.GetRootObject(), 
                typeof(Transform), allowSceneObjects: true);                
            t.SetRootObject(rootObject);
            
            EditorGUILayout.Space();
        }
    }

    private void DrawInstanceSettings(MeshSyncServer t)
    {
        var style = EditorStyles.foldout;
        style.fontStyle = FontStyle.Bold;
        t.foldInstanceSettings = EditorGUILayout.Foldout(t.foldInstanceSettings, "Instances", true, style);
        if (t.foldInstanceSettings)
        { 
            EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Instance Camera mode",
                guiFunc: () => EditorGUILayout.Popup(new GUIContent("Camera mode"),
                    (int)t.cameraMode, MeshSyncEditorConstants.INSTANCE_CAMERA_MODE_ENUMS),
                updateFunc: (int val) => { t.cameraMode = (MeshSyncInstanceRenderer.CameraMode)val; }
            );

        }
        
    }

//----------------------------------------------------------------------------------------------------------------------                
    private MeshSyncServer m_meshSyncServer = null;        
}

} //end namespace
