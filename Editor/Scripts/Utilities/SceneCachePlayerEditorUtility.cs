﻿using System;
using System.Collections.Generic;
using System.IO;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using UnityEditor;
using UnityEngine;
using UnityEngine.Analytics;
using UnityEngine.Animations;
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

        //Optimize serialization by ensuring to serialize the key values at the end 
        player.EnableKeyValuesSerialization(false);
        prefab = player.gameObject.SaveAsPrefab(prefabPath);
        if (null == prefab) {
            Object.DestroyImmediate(go);            
            return false;
        }
        
        player.EnableKeyValuesSerialization(true);       
        PrefabUtility.ApplyPrefabInstance(player.gameObject, InteractionMode.AutomatedAction);
        
        Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    internal static void ChangeSceneCacheFile(SceneCachePlayer cachePlayer, string sceneCacheFilePath) {
        string     prefabPath = null;
        GameObject go         = cachePlayer.gameObject;
        //Check if it's possible to reuse the old assetsFolder
        string assetsFolder = cachePlayer.GetAssetsFolder();
        if (string.IsNullOrEmpty(assetsFolder)) {
            MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();        
            string                  scOutputPath    = projectSettings.GetSceneCacheOutputPath();            
            assetsFolder = Path.Combine(scOutputPath, Path.GetFileNameWithoutExtension(sceneCacheFilePath));
        }
        
        bool isPrefabInstance = cachePlayer.gameObject.IsPrefabInstance();        
        //We are directly modifying a prefab
        if (!isPrefabInstance && go.IsPrefab()) {
            prefabPath = AssetDatabase.GetAssetPath(go);
            CreateSceneCachePlayerAndPrefab(sceneCacheFilePath, prefabPath, assetsFolder, out SceneCachePlayer player,
                out GameObject newPrefab);
            Object.DestroyImmediate(player.gameObject);
            return;

        } 
        
        if (isPrefabInstance) {
            GameObject prefab = PrefabUtility.GetCorrespondingObjectFromSource(cachePlayer.gameObject);
            
            //GetCorrespondingObjectFromSource() may return the ".sc" GameObject instead of the prefab
            //due to the SceneCacheImporter
            string assetPath = AssetDatabase.GetAssetPath(prefab);
            if (Path.GetExtension(assetPath).ToLower() == ".prefab") {
                prefabPath = assetPath;
            }            
        } 
        
        cachePlayer.CloseCache();
        
        //[TODO-sin: 2020-9-28] Find out if it is possible to do undo properly
        Undo.RegisterFullObjectHierarchyUndo(cachePlayer.gameObject, "SceneCachePlayer");

        Dictionary<string,EntityRecord> prevRecords = new Dictionary<string, EntityRecord>(cachePlayer.GetClientObjects());                
        if (isPrefabInstance) {
            PrefabUtility.UnpackPrefabInstance(cachePlayer.gameObject, PrefabUnpackMode.Completely, InteractionMode.AutomatedAction);            
            
        }


        UpdateEntityHandler doExport2 = (GameObject updatedGo, TransformData data) => {
            UpdateEntityHandler(updatedGo, data, prevRecords);
        };

        
        cachePlayer.onUpdateEntity += doExport2;
        
        cachePlayer.Init(assetsFolder);
        cachePlayer.OpenCacheInEditor(sceneCacheFilePath);        
        IDictionary<string,EntityRecord> curRecords = cachePlayer.GetClientObjects();

        cachePlayer.onUpdateEntity -= doExport2;
        
        UpdateGameObjectsByComparingRecords(prevRecords, curRecords);

        if (string.IsNullOrEmpty(prefabPath)) {
            return;
        }
        
        //Save as prefab again if it was truly a prefab instance
        cachePlayer.gameObject.SaveAsPrefab(prefabPath);
    }

    internal static void ReloadSceneCacheFile(SceneCachePlayer cachePlayer) {
        string sceneCacheFilePath = cachePlayer.GetSceneCacheFilePath();        
        ChangeSceneCacheFile(cachePlayer, sceneCacheFilePath);
        
    }

    private static void UpdateEntityHandler(GameObject obj, TransformData data, IDictionary<string, EntityRecord> prevRecords) 
    {
        if (!prevRecords.ContainsKey(data.path))
            return;

        EntityRecord prevRecord = prevRecords[data.path];

        if (data.entityType == prevRecord.dataType)
            return;

        HashSet<Type> componentTypes = new HashSet<Type>() {
            typeof(SkinnedMeshRenderer),             //check bones
            typeof(MeshFilter),            
            typeof(MeshRenderer),            
            typeof(PointCacheRenderer),
            typeof(PointCache),
            typeof(Camera),
            typeof(Light),            
//            typeof(HDAdditionalLightData),            

            //Constraints
            typeof(AimConstraint),
            typeof(ParentConstraint),
            typeof(PositionConstraint),
            typeof(RotationConstraint),
            typeof(ScaleConstraint),
            
            //Animator
            typeof(MeshCollider),
            
        };

        //Check which component should remain
        switch (data.entityType) {
            case EntityType.Camera:
                componentTypes.Remove(typeof(Camera));
                break;
            case EntityType.Light:
                componentTypes.Remove(typeof(Light));
                break;
            case EntityType.Mesh: {
                componentTypes.Remove(typeof(SkinnedMeshRenderer));
                componentTypes.Remove(typeof(MeshFilter));
                componentTypes.Remove(typeof(MeshRenderer));
                break;                
            }                
            case EntityType.Points:
                componentTypes.Remove(typeof(PointCache));
                componentTypes.Remove(typeof(PointCacheRenderer));
                break;
            
        }
        

        foreach (Type c in componentTypes) {
            var a = obj.GetComponent(c);
            if (null == a)
                continue;
            
            ObjectUtility.Destroy(a);
        }

    }
    
    //Delete GameObject if they exist in prevRecords, but not in curRecords
    //Update them if the type is different
    private static void UpdateGameObjectsByComparingRecords(IDictionary<string, EntityRecord> prevRecords,
        IDictionary<string, EntityRecord> curRecords) 
    {
        foreach (KeyValuePair<string, EntityRecord> kv in prevRecords) {
            string       goPath           = kv.Key;
            EntityRecord prevEntityRecord = kv.Value;

            if (!curRecords.ContainsKey(goPath)) {
                ObjectUtility.Destroy(prevEntityRecord.go);
                continue;
            }

            EntityRecord curEntityRecord = curRecords[goPath];
            if (prevEntityRecord.dataType == curEntityRecord.dataType)
                continue;

            //Update entity

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
        
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();
        string                  scOutputPath    = projectSettings.GetSceneCacheOutputPath();
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