using System.IO;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {


internal static class MeshSyncMenu  {



    //----------------------------------------------------------------------------------------------------------------------    
    #region Server
    [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
    internal static MeshSyncServer CreateMeshSyncServerMenu(MenuCommand menuCommand)
    {
        MeshSyncServer mss = CreateMeshSyncServer(true);
        if (mss != null)
            Undo.RegisterCreatedObjectUndo(mss.gameObject, "MeshSyncServer");
        Selection.activeTransform = mss.transform;

        return mss;
    }

    internal static MeshSyncServer CreateMeshSyncServer(bool autoStart) {
        GameObject     go  = new GameObject("MeshSyncServer");
        MeshSyncServer mss = go.AddComponent<MeshSyncServer>();
        mss.Init(MeshSyncConstants.DEFAULT_ASSETS_PATH);
        mss.SetAutoStartServer(autoStart);
        return mss;
    }
    #endregion
    
//----------------------------------------------------------------------------------------------------------------------
    
    #region SceneCache

    [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
    static void CreateSceneCachePlayerMenu(MenuCommand menuCommand) {
        string sceneCacheFilePath = EditorUtility.OpenFilePanelWithFilters("Select Cache File", "",
            new string[]{ "All supported files", "sc", "All files", "*" });

        if (string.IsNullOrEmpty(sceneCacheFilePath)) {
            return;
        }

        //Prefab and assets path
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();
        
        string scOutputPath    = projectSettings.GetSceneCacheOutputPath();
        string prefabFileName = Path.GetFileNameWithoutExtension(sceneCacheFilePath);
        string prefabPath = $"{scOutputPath}/{prefabFileName}.prefab";
        string assetsFolder = Path.Combine(scOutputPath, prefabFileName);

        bool created = SceneCachePlayerEditorUtility.CreateSceneCachePlayerAndPrefab(sceneCacheFilePath, prefabPath, 
            assetsFolder, out SceneCachePlayer player, out GameObject prefab);        
       
        
        if (!created) {
            EditorUtility.DisplayDialog("MeshSync"
                ,"Failed to open " + sceneCacheFilePath 
                    + ". Possible reasons: file format version does not match, or the file is not scene cache."
                ,"Ok"                
            );
        } 
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    
    #endregion //SceneCache
}

} //end namespace

