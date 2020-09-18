using System;
using System.IO;
using NUnit.Framework;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {

[CustomEditor(typeof(SceneCachePlayer))]
internal class SceneCachePlayerEditor : MeshSyncPlayerEditor {
    [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
    public static void CreateSceneCachePlayerMenu(MenuCommand menuCommand) {
        string path = EditorUtility.OpenFilePanelWithFilters("Select Cache File", "",
            new string[]{ "All supported files", "sc", "All files", "*" });
        if (path.Length > 0) {
            GameObject go = CreateSceneCachePlayerPrefab(path);
            if (go != null)
                Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
        }
    }
    
//----------------------------------------------------------------------------------------------------------------------    

    public static GameObject CreateSceneCachePlayer(string path) {
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
        
        string                  assetDir       =  Path.Combine(scOutputPath.Substring(numAssetsChars), go.name);
        
        SceneCachePlayer        player          = go.AddComponent<SceneCachePlayer>();
        player.rootObject = go.GetComponent<Transform>();
        player.assetDir = new DataPath(DataPath.Root.DataPath, assetDir);
        player.markMeshesDynamic = true;
        player.dontSaveAssetsInScene = true;

        if (!player.OpenCache(path)) {
            Debug.LogError("Failed to open " + path + ". Possible reasons: file format version does not match, or the file is not scene cache.");
            DestroyImmediate(go);
            return null;
        }
        return go;
    }

//----------------------------------------------------------------------------------------------------------------------    
    public static GameObject CreateSceneCachePlayerPrefab(string path) {
        
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
    
    public override void OnInspectorGUI() {
        SerializedObject so = serializedObject;
        SceneCachePlayer t = target as SceneCachePlayer;

        EditorGUILayout.Space();
        DrawCacheSettings(t, so);
        DrawPlayerSettings(t, so);
        MeshSyncPlayerConfig config = t.GetConfig();
        if (config.Profiling) {
            EditorGUILayout.TextArea(t.dbgProfileReport, GUILayout.Height(120));
            EditorGUILayout.Space();
        }

        DrawMaterialList(t);
        DrawTextureList(t);
        DrawAnimationTweak(t);
        DrawExportAssets(t);
        DrawPluginVersion();

        so.ApplyModifiedProperties();
    }

//----------------------------------------------------------------------------------------------------------------------
    
    void DrawCacheSettings(SceneCachePlayer t, SerializedObject so) {
        GUIStyle styleFold = EditorStyles.foldout;
        styleFold.fontStyle = FontStyle.Bold;

        t.foldCacheSettings = EditorGUILayout.Foldout(t.foldCacheSettings, "Player", true, styleFold);
        if (t.foldCacheSettings) {
            // cache file path
            EditorGUILayout.PropertyField(so.FindProperty("m_cacheFilePath"));
            DataPath dataPath = t.cacheFilePath;
            if (dataPath.GetRoot() != DataPath.Root.StreamingAssets) {
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Copy to StreamingAssets", GUILayout.Width(160.0f))) {
                    string dstPath = Misc.CopyFileToStreamingAssets(dataPath.GetFullPath());
                    Undo.RecordObject(t, "SceneCachePlayer");
                    t.OpenCache(dstPath);
                    Repaint();
                }
                if (GUILayout.Button("Move to StreamingAssets", GUILayout.Width(160.0f))) {
                    t.CloseCache();
                    string dstPath = Misc.MoveFileToStreamingAssets(dataPath.GetFullPath());
                    Undo.RecordObject(t, "SceneCachePlayer");
                    t.OpenCache(dstPath);
                    Repaint();
                }
                GUILayout.EndHorizontal();
            }
            EditorGUILayout.Space();


            // time / frame
            System.Action resetTimeAnimation = () => {
                so.ApplyModifiedProperties();
                t.ResetTimeAnimation();
            };

            EditorGUI.BeginChangeCheck();
            EditorGUILayout.PropertyField(so.FindProperty("m_timeUnit"));
            if (EditorGUI.EndChangeCheck())
                resetTimeAnimation();

            if (t.timeUnit == SceneCachePlayer.TimeUnit.Seconds) {
                EditorGUILayout.PropertyField(so.FindProperty("m_time"));
                EditorGUILayout.PropertyField(so.FindProperty("m_interpolation"));
            } else if (t.timeUnit == SceneCachePlayer.TimeUnit.Frames) {
                EditorGUI.BeginChangeCheck();
                EditorGUILayout.PropertyField(so.FindProperty("m_baseFrame"));
                if (EditorGUI.EndChangeCheck())
                    resetTimeAnimation();

                EditorGUILayout.PropertyField(so.FindProperty("m_frame"));
            }

            // preload
            {
                SerializedProperty preloadLength = so.FindProperty("m_preloadLength");
                preloadLength.intValue = EditorGUILayout.IntSlider("Preload Length", preloadLength.intValue, 0, t.frameCount);
            }

            EditorGUILayout.Space();
        }
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
