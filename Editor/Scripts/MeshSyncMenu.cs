﻿using System;
using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEngine;
using UnityEngine.Assertions;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor {


internal static class MeshSyncMenu  {

    
    //[MenuItem("Assets/MeshSync/Download DCC Plugins", false, 100)]
    static void DownloadDCCPlugins() {

        //Actual plugin name: UnityMeshSync_<version>_<postfix>
        string[] dccPlatformNames = {
            "3DSMAX_Windows.zip",
            "Blender_Linux.zip",
            "Blender_Mac.zip",
            "Blender_Windows.zip",
            "Maya_Linux.zip",
            "Maya_Mac.zip",
            "Maya_Windows.zip",
            "Metasequoia_Windows.zip",
            "MotionBuilder_Linux.zip",
            "MotionBuilder_Windows.zip" 
        };

        string destFolder = EditorUtility.OpenFolderPanel("Select copy destination", "", "");
        if (string.IsNullOrEmpty(destFolder))
            return;

        
        DCCPluginDownloader downloader = new DCCPluginDownloader(true, destFolder, dccPlatformNames);
        downloader.Execute((string version, List<string> dccPluginLocalPaths) => {
            Debug.Log("Downloaded " + dccPluginLocalPaths.Count 
                       + "MeshSync DCC Plugins to " + destFolder + " Version: " + version);
            EditorUtility.RevealInFinder(destFolder);
        }, () => {
            Debug.LogError("Failed to download MeshSync DCC plugins.");
        });
        
    }

//----------------------------------------------------------------------------------------------------------------------    
    #region Server
    [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
    internal static void CreateMeshSyncServerMenu(MenuCommand menuCommand) {
        MeshSyncServer mss = CreateMeshSyncServer(true);
        if (mss != null)
            Undo.RegisterCreatedObjectUndo(mss.gameObject, "MeshSyncServer");
        Selection.activeTransform = mss.transform;
    }

    internal static MeshSyncServer CreateMeshSyncServer(bool autoStart) {
        GameObject     go  = new GameObject("MeshSyncServer");
        MeshSyncServer mss = go.AddComponent<MeshSyncServer>();
        mss.Init("Assets/MeshSyncAssets");
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
        
        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        string                  scOutputPath    = runtimeSettings.GetSceneCacheOutputPath();

        GameObject go = new GameObject();        
        go.name = System.IO.Path.GetFileNameWithoutExtension(sceneCacheFilePath);

        string prefabPath = $"{scOutputPath}/{go.name}.prefab";
        bool prefabCreated = CreateSceneCachePlayerPrefab(go, sceneCacheFilePath, prefabPath);
        if (!prefabCreated) {
            EditorUtility.DisplayDialog("MeshSync"
                ,"Failed to open " + sceneCacheFilePath 
                    + ". Possible reasons: file format version does not match, or the file is not scene cache."
                ,"Ok"                
            );
            Object.DestroyImmediate(go);            
        } else {           
            Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");            
        }
    }
    
//----------------------------------------------------------------------------------------------------------------------    

    private static SceneCachePlayer  CreateSceneCachePlayer(GameObject go, string sceneCacheFilePath) {
        if (!ValidateSceneCacheOutputPath()) {
            return null;
        }
        
        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        string                  scOutputPath    = runtimeSettings.GetSceneCacheOutputPath();
       
        string assetsFolder = Path.Combine(scOutputPath, go.name);        
        SceneCachePlayer player = go.GetOrAddComponent<SceneCachePlayer>();
        player.Init(assetsFolder);

        if (!player.OpenCache(sceneCacheFilePath)) {
            return null;
        }
        
        // Further initialization after opening cache
        player.UpdatePlayer();
        player.ExportMaterials(false, true);
        player.ResetTimeAnimation();
        player.handleAssets = false;
        
        SceneData scene = player.GetLastScene();
        if (!scene.submeshesHaveUniqueMaterial) {
            MeshSyncPlayerConfig config = player.GetConfig();
            config.SyncMaterialList = false;
        }
        
        return player;
    }

//----------------------------------------------------------------------------------------------------------------------    

    internal static bool CreateSceneCachePlayerPrefab(GameObject go, string sceneCacheFilePath, string prefabPath) {
        Assert.IsNotNull(go);
        go.DestroyChildrenImmediate();
        
        if (!ValidateSceneCacheOutputPath()) {
            return false;
        }
        
        SceneCachePlayer player = CreateSceneCachePlayer(go, sceneCacheFilePath);
        if (null==player)
            return false;

        PrefabUtility.SaveAsPrefabAssetAndConnect(go, prefabPath, InteractionMode.AutomatedAction);
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------    

    private static bool ValidateSceneCacheOutputPath() {
        
        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        string                  scOutputPath    = runtimeSettings.GetSceneCacheOutputPath();
        if (!scOutputPath.StartsWith("Assets")) {
            DisplaySceneCacheOutputPathErrorDialog(scOutputPath);
            return false;            
        }        
        
        try {
            Directory.CreateDirectory(scOutputPath);
        } catch {
            DisplaySceneCacheOutputPathErrorDialog(scOutputPath);
            return false;
        }

        return true;
    }

    private static void DisplaySceneCacheOutputPathErrorDialog(string path) {
        EditorUtility.DisplayDialog("MeshSync",
            $"Invalid SceneCache output path: {path}. " + Environment.NewLine + 
            "Please configure in ProjectSettings", 
            "Ok");
        
    }
    
    #endregion //SceneCache
}

} //end namespace

