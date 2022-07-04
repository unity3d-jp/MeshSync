using System.Collections.Generic;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {
[CustomEditor(typeof(MeshSyncServer))]
[InitializeOnLoad]
internal class MeshSyncServerInspector : BaseMeshSyncInspector {
#if AT_USE_PROBUILDER
    static System.Reflection.FieldInfo versionInfo = typeof(UnityEngine.ProBuilder.ProBuilderMesh).GetField("m_VersionIndex", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);

    static Dictionary<UnityEngine.ProBuilder.ProBuilderMesh, ushort> versionIndices = new Dictionary<UnityEngine.ProBuilder.ProBuilderMesh, ushort>();

    static MeshSyncServerInspector()
    {
        BaseMeshSync.ProBuilderBeforeRebuild += ProBuilderBeforeRebuild;
        BaseMeshSync.ProBuilderAfterRebuild += ProBuilderAfterRebuild;
    }

    private static void MeshesChanged(IEnumerable<UnityEngine.ProBuilder.ProBuilderMesh> meshes) {
        if (meshes == null) {
            return;
        }

        foreach (var mesh in meshes) {
            var version = (ushort)versionInfo.GetValue(mesh);

            if (!versionIndices.TryGetValue(mesh, out var storedVersion)) {
                versionIndices.Add(mesh, version);
            }
            else if (version != storedVersion) {
                versionIndices[mesh] = version;

                var server = mesh.GetComponentInParent<MeshSyncServer>();

                server?.MeshChanged(mesh);
            }
        }
    }

    private static void ProBuilderEditor_afterMeshModification(IEnumerable<UnityEngine.ProBuilder.ProBuilderMesh> meshes)
    {      
        MeshesChanged(meshes);
    }

    private static void ProBuilderEditor_selectionUpdated(IEnumerable<UnityEngine.ProBuilder.ProBuilderMesh> meshes)
    {
        if (!UnityEditorInternal.InternalEditorUtility.isApplicationActive)
        {
            return;
        }

        MeshesChanged(meshes);
    }

    private static void ProBuilderBeforeRebuild()
    {
        // Change select mode back and forth to ensure probuilder invalidates its internal cache:
        var t = UnityEditor.ProBuilder.ProBuilderEditor.selectMode;
        UnityEditor.ProBuilder.ProBuilderEditor.selectMode = UnityEngine.ProBuilder.SelectMode.Object;
        UnityEditor.ProBuilder.ProBuilderEditor.selectMode = t;

        // Init pro builder callbacks:
        UnityEditor.ProBuilder.ProBuilderEditor.afterMeshModification -= ProBuilderEditor_afterMeshModification;
        UnityEditor.ProBuilder.ProBuilderEditor.selectionUpdated -= ProBuilderEditor_selectionUpdated;
    }

    private static void ProBuilderAfterRebuild()
    {
        // Init pro builder callbacks:
        UnityEditor.ProBuilder.ProBuilderEditor.afterMeshModification += ProBuilderEditor_afterMeshModification;
        UnityEditor.ProBuilder.ProBuilderEditor.selectionUpdated += ProBuilderEditor_selectionUpdated;
    }
#endif

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
        DrawExportAssets(m_meshSyncServer);
        DrawInstanceSettings(m_meshSyncServer);
        DrawDCCToolInfo(m_meshSyncServer);
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
            t.InstanceHandling = (BaseMeshSync.InstanceHandlingType)EditorGUILayout.EnumPopup("Instance handling", t.InstanceHandling);

            DrawPrefabListElement(t);
        }

        EditorGUILayout.LabelField($"Instance count: {t.InstanceCount}");
    }

    static void DrawPrefabListElement(MeshSyncServer t)
    {
        if (t.prefabList.Count > 0)
        {
            EditorGUILayout.LabelField("Instance prefabs:");

            EditorGUI.indentLevel++;
            foreach (var prefabHolder in t.prefabList)
            {
                EditorGUILayout.ObjectField(prefabHolder.name, prefabHolder.prefab, typeof(GameObject), true);
            }
            EditorGUI.indentLevel--;

            if (GUILayout.Button("Clear prefabs"))
            {
                t.ClearInstancePrefabs();
            }
        }
    }

    static void DrawDCCToolInfo(MeshSyncServer server)
    {
        if (server != null)
        {
            GUILayout.BeginHorizontal();

            var newAsset = EditorGUILayout.ObjectField("DCC asset file:",
                server.m_DCCAsset,
                typeof(UnityEngine.Object), true);

            if (newAsset != server.m_DCCAsset)
            {
                server.m_DCCAsset = newAsset;
                server.m_DCCInterop = MeshSyncServerInspectorUtils.GetLauncherForAsset(server.m_DCCAsset);
            }

            if (server.m_DCCAsset != null)
            {
                if (GUILayout.Button("Live Edit"))
                {
                    GUILayout.EndHorizontal();
                    MeshSyncServerInspectorUtils.OpenDCCAsset(server);
                    return;
                }
            }

            GUILayout.EndHorizontal();

            if (server.m_DCCInterop == null)
            {
                server.m_DCCInterop = MeshSyncServerInspectorUtils.GetLauncherForAsset(server.m_DCCAsset);
            }

            server.m_DCCInterop?.DrawDCCMenu(server);
        }
    }

//----------------------------------------------------------------------------------------------------------------------                
    private MeshSyncServer m_meshSyncServer = null;        
}

} //end namespace
