using System;
using System.Collections.Generic;
using System.IO;
using NUnit.Framework;
using UnityEditor;
using UnityEngine;

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

    [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
    static void CreateSceneCachePlayerMenu(MenuCommand menuCommand) {
        string path = EditorUtility.OpenFilePanelWithFilters("Select Cache File", "",
            new string[]{ "All supported files", "sc", "All files", "*" });
        if (path.Length > 0) {
            GameObject go = CreateSceneCachePlayerPrefab(path);
            if (go != null)
                Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
        }
    }
    
//----------------------------------------------------------------------------------------------------------------------    

    internal static GameObject CreateSceneCachePlayer(string path) {
        if (!ValidateOutputPath()) {
            return null;
        }
        
        // create temporary GO to make prefab
        GameObject go = new GameObject();
        go.name = System.IO.Path.GetFileNameWithoutExtension(path);

        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        string                  scOutputPath    = runtimeSettings.GetSceneCacheOutputPath();

        int numAssetsChars = "Assets".Length;
        Assert.IsTrue(scOutputPath.Length >= numAssetsChars);
        
        string assetDir =  Path.Combine(scOutputPath.Substring(numAssetsChars), go.name);
        
        SceneCachePlayer player = go.AddComponent<SceneCachePlayer>();
        player.rootObject            = go.GetComponent<Transform>();
        player.assetDir              = new DataPath(DataPath.Root.DataPath, assetDir);
        player.markMeshesDynamic     = true;
        player.dontSaveAssetsInScene = true;

        if (!player.OpenCache(path)) {
            Debug.LogError("Failed to open " + path + ". Possible reasons: file format version does not match, or the file is not scene cache.");
            UnityEngine.Object.DestroyImmediate(go);
            return null;
        }
        return go;
    }

//----------------------------------------------------------------------------------------------------------------------    

    internal static GameObject CreateSceneCachePlayerPrefab(string path) {
        
        if (!ValidateOutputPath()) {
            return null;
        }
        
        GameObject go = CreateSceneCachePlayer(path);
        if (go == null)
            return null;

        // export materials & animation and generate prefab
        SceneCachePlayer player = go.GetComponent<SceneCachePlayer>();
        player.UpdatePlayer();
        player.ExportMaterials(false, true);
        player.ResetTimeAnimation();
        player.handleAssets = false;
        SceneData scene = player.GetLastScene();
        if (!scene.submeshesHaveUniqueMaterial) {
            MeshSyncPlayerConfig config = player.GetConfig();
            config.SyncMaterialList = false;
        }


       
        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        string                  scOutputPath    = runtimeSettings.GetSceneCacheOutputPath();
        
        string prefabPath = $"{scOutputPath}/{go.name}.prefab";
        PrefabUtility.SaveAsPrefabAssetAndConnect(go, prefabPath, InteractionMode.AutomatedAction);
        return go;
    }

//----------------------------------------------------------------------------------------------------------------------    

    static bool ValidateOutputPath() {
        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        string                  scOutputPath    = runtimeSettings.GetSceneCacheOutputPath();
        try {
            Directory.CreateDirectory(scOutputPath);
        } catch {
            EditorUtility.DisplayDialog("MeshSync",
                $"Invalid SceneCache output path: {scOutputPath}. " + Environment.NewLine + 
                "Please configure in ProjectSettings", 
                "Ok");
            return false;
        }

        return true;
    }
    
}

} //end namespace

