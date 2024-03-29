using System.IO;
using Unity.MeshSync.Editor.Analytics;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal static class MeshSyncMenu  {
    // Menu strings:
    private const string menuItem_LiveEditInBlender = "Assets/MeshSync/Live-edit in blender";

//----------------------------------------------------------------------------------------------------------------------    

    #region Server

    [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
    internal static MeshSyncServer CreateMeshSyncServerMenu(MenuCommand menuCommand) {
        MeshSyncServer mss = CreateMeshSyncServer(true);
        if (mss != null)
            Undo.RegisterCreatedObjectUndo(mss.gameObject, "MeshSyncServer");
        Selection.activeTransform = mss.transform;

        return mss;
    }

    internal static MeshSyncServer CreateMeshSyncServer(bool autoStart) {
        GameObject     go  = new GameObject("MeshSyncServer");
        MeshSyncServer mss = go.AddComponent<MeshSyncServer>();

        // Subscribe analytics event observer to MehSyncServer
        MeshSyncObserver observer = new MeshSyncObserver();
        mss.Subscribe(observer);


        mss.Init(MeshSyncConstants.DEFAULT_ASSETS_PATH);
        mss.SetAutoStartServer(autoStart);
        return mss;
    }

    [MenuItem(menuItem_LiveEditInBlender)]
    private static void LiveEditInBlender() {
        Object selectedAsset = Selection.objects[0];

        MeshSyncServer[] servers = Object.FindObjectsOfType<MeshSyncServer>();

        MeshSyncServer server = null;

        foreach (MeshSyncServer serverInScene in servers)
            if (serverInScene.enabled) {
                server = serverInScene;
                break;
            }

        if (server == null) server = CreateMeshSyncServerMenu(null);

        server.DCCAsset = selectedAsset;

        MeshSyncServerInspectorUtils.OpenDCCAsset(server);
    }

    [MenuItem(menuItem_LiveEditInBlender, validate = true)]
    private static bool CanLiveEditInBlender() {
        if (Selection.objects.Length == 1) {
            string assetPath = AssetDatabase.GetAssetPath(Selection.objects[0]);
            if (assetPath != null && assetPath.EndsWith(BlenderLauncher.FileFormat)) return true;
        }

        return false;
    }

    #endregion

//----------------------------------------------------------------------------------------------------------------------

    #region SceneCache

    [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
    private static void CreateSceneCachePlayerMenu(MenuCommand menuCommand) {
        string sceneCacheFilePath = EditorUtility.OpenFilePanelWithFilters("Select Cache File", "",
            new string[] { "All supported files", "sc", "All files", "*" });

        if (string.IsNullOrEmpty(sceneCacheFilePath)) return;

        //Prefab and assets path
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateInstance();

        string scOutputPath   = projectSettings.GetSceneCacheOutputPath();
        string prefabFileName = Path.GetFileNameWithoutExtension(sceneCacheFilePath);
        string prefabPath     = $"{scOutputPath}/{prefabFileName}.prefab";
        string assetsFolder   = Path.Combine(scOutputPath, prefabFileName);

        bool created = SceneCachePlayerEditorUtility.CreateSceneCachePlayerAndPrefab(sceneCacheFilePath, prefabPath,
            assetsFolder, out SceneCachePlayer player, out GameObject prefab);


        if (!created)
            EditorUtility.DisplayDialog("MeshSync"
                , "Failed to open " + sceneCacheFilePath
                + ". Possible reasons: file format version does not match, or the file is not scene cache."
                , "Ok"
            );
    }

//----------------------------------------------------------------------------------------------------------------------    

    #endregion //SceneCache
}
} //end namespace