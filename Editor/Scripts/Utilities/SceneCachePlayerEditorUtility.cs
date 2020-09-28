﻿using System;
using System.IO;
using UnityEditor;
using UnityEngine;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor  {

internal static class SceneCachePlayerEditorUtility {

    internal static bool CreateSceneCachePlayerAndPrefab(string sceneCacheFilePath, 
        string prefabPath, string assetsFolder, 
        out SceneCachePlayer player, out GameObject prefab) 
    {
        player = null;
        prefab = null;
        
        GameObject go = new GameObject();        
        go.name = Path.GetFileNameWithoutExtension(sceneCacheFilePath);

        player = AddSceneCachePlayer(go, sceneCacheFilePath, assetsFolder);
        if (null == player) {
            Object.DestroyImmediate(go);            
            return false;
        }
        prefab = player.gameObject.SaveAsPrefab(prefabPath);
        if (null == prefab) {
            Object.DestroyImmediate(go);            
            return false;
        }
        
        Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    internal static void ChangeSceneCacheFile(SceneCachePlayer cachePlayer, string sceneCacheFilePath) {
        string prefabPath = null;
        if (cachePlayer.gameObject.IsPrefabInstance()) {
            GameObject prefab = PrefabUtility.GetCorrespondingObjectFromSource(cachePlayer.gameObject);
            prefabPath = AssetDatabase.GetAssetPath(prefab);
        }
        
        cachePlayer.CloseCache();
        Undo.RecordObject(cachePlayer, "SceneCachePlayer");

        //Check if it's possible to reuse the old assetsFolder
        string assetsFolder = cachePlayer.GetAssetsFolder();
        if (string.IsNullOrEmpty(assetsFolder)) {
            MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();        
            string                  scOutputPath    = runtimeSettings.GetSceneCacheOutputPath();            
            assetsFolder = Path.Combine(scOutputPath, Path.GetFileNameWithoutExtension(sceneCacheFilePath));
        }
        
        cachePlayer.Init(assetsFolder);
        cachePlayer.OpenCacheInEditor(sceneCacheFilePath);

        //Save as prefab again
        if (!string.IsNullOrEmpty(prefabPath)) {
            cachePlayer.gameObject.SaveAsPrefab(prefabPath);
        }
        
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    private static SceneCachePlayer  AddSceneCachePlayer(GameObject go, 
                                                            string sceneCacheFilePath, 
                                                            string assetsFolder) 
    {
        if (!ValidateSceneCacheOutputPath()) {
            return null;
        }
        
              
        SceneCachePlayer player = go.GetOrAddComponent<SceneCachePlayer>();
        player.Init(assetsFolder);

        if (!player.OpenCacheInEditor(sceneCacheFilePath)) {
            return null;
        }       
        
        return player;
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
    
    
}

} //end namespace