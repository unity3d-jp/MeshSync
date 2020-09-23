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

    public override void OnEnable() {
        base.OnEnable();
        m_sceneCachePlayer = target as SceneCachePlayer;
        
    }


//----------------------------------------------------------------------------------------------------------------------
    
    
    
    public override void OnInspectorGUI() {
        SerializedObject so = serializedObject;

        EditorGUILayout.Space();
        DrawCacheSettings(so);
        DrawPlayerSettings(m_sceneCachePlayer, so);
        MeshSyncPlayerConfig config = m_sceneCachePlayer.GetConfig();
        if (config.Profiling) {
            EditorGUILayout.TextArea(m_sceneCachePlayer.dbgProfileReport, GUILayout.Height(120));
            EditorGUILayout.Space();
        }

        DrawMaterialList(m_sceneCachePlayer);
        DrawTextureList(m_sceneCachePlayer);
        DrawAnimationTweak(m_sceneCachePlayer);
        DrawExportAssets(m_sceneCachePlayer);
        DrawPluginVersion();

        so.ApplyModifiedProperties();
    }

//----------------------------------------------------------------------------------------------------------------------
    
    void DrawCacheSettings(SerializedObject so) {
        GUIStyle styleFold = EditorStyles.foldout;
        styleFold.fontStyle = FontStyle.Bold;

        m_sceneCachePlayer.foldCacheSettings = EditorGUILayout.Foldout(m_sceneCachePlayer.foldCacheSettings, "Player", true, styleFold);
        if (m_sceneCachePlayer.foldCacheSettings) {
            // cache file path
            string fullPath = m_sceneCachePlayer.GetFilePath();
            using (new EditorGUI.DisabledScope(true)) {               
                EditorGUILayout.TextField("Cache File Path", AssetUtility.NormalizeAssetPath(fullPath));
            }
            if (!fullPath.StartsWith(Application.streamingAssetsPath)) {
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                const float BUTTON_WIDTH = 50.0f;
                if (GUILayout.Button("Copy", GUILayout.Width(BUTTON_WIDTH))) {
                    string dstPath = Misc.CopyFileToStreamingAssets(fullPath);
                    OnFilePathChanged(m_sceneCachePlayer, dstPath);
                }
                GUILayout.Label("or");
                if (GUILayout.Button("Move", GUILayout.Width(BUTTON_WIDTH))) {
                    m_sceneCachePlayer.CloseCache();
                    string dstPath = Misc.MoveFileToStreamingAssets(fullPath);
                    OnFilePathChanged(m_sceneCachePlayer, dstPath);
                }
                GUILayout.Label("to StreamingAssets");
                GUILayout.EndHorizontal();
            }
            EditorGUILayout.Space();

            // time / frame
            System.Action resetTimeAnimation = () => {
                so.ApplyModifiedProperties();
                m_sceneCachePlayer.ResetTimeAnimation();
            };

            EditorGUI.BeginChangeCheck();
            EditorGUILayout.PropertyField(so.FindProperty("m_timeUnit"));
            if (EditorGUI.EndChangeCheck())
                resetTimeAnimation();

            if (m_sceneCachePlayer.timeUnit == SceneCachePlayer.TimeUnit.Seconds) {
                EditorGUILayout.PropertyField(so.FindProperty("m_time"));
                EditorGUILayout.PropertyField(so.FindProperty("m_interpolation"));
            } else if (m_sceneCachePlayer.timeUnit == SceneCachePlayer.TimeUnit.Frames) {
                EditorGUI.BeginChangeCheck();
                EditorGUILayout.PropertyField(so.FindProperty("m_baseFrame"));
                if (EditorGUI.EndChangeCheck())
                    resetTimeAnimation();

                EditorGUILayout.PropertyField(so.FindProperty("m_frame"));
            }

            // preload
            {
                SerializedProperty preloadLength = so.FindProperty("m_preloadLength");
                preloadLength.intValue = EditorGUILayout.IntSlider("Preload Length", preloadLength.intValue, 0, m_sceneCachePlayer.frameCount);
            }

            EditorGUILayout.Space();
        }
    }

//----------------------------------------------------------------------------------------------------------------------

    void OnFilePathChanged(SceneCachePlayer cachePlayer, string path) {
        cachePlayer.SetFilePath(path);
        Undo.RecordObject(cachePlayer, "SceneCachePlayer");
        cachePlayer.OpenCache(path);
        Repaint();        
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


//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer m_sceneCachePlayer = null;



}

} //end namespace
