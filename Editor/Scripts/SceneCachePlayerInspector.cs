using System.Collections.Generic;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {

[CustomEditor(typeof(SceneCachePlayer))]
[CanEditMultipleObjects]
internal class SceneCachePlayerInspector : MeshSyncPlayerInspector {
    

//----------------------------------------------------------------------------------------------------------------------

    public override void OnEnable() {
        base.OnEnable();
        m_sceneCachePlayer = target as SceneCachePlayer;
        
        m_targets.Clear();
        foreach (Object singleTarget in targets) {
            m_targets.Add(singleTarget as SceneCachePlayer);                
        }        
    }


//----------------------------------------------------------------------------------------------------------------------
    
    
    
    public override void OnInspectorGUI() {

        int numTargets = m_targets.Count;
        if (numTargets >= 2) {
            ShowReloadSelectedSceneCacheFilesGUI();
            return;
        }
        
        EditorGUILayout.Space();
        DrawCacheSettings();
        DrawPlayerSettings(m_sceneCachePlayer);
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

        PrefabUtility.RecordPrefabInstancePropertyModifications(m_sceneCachePlayer);
        
    }

//----------------------------------------------------------------------------------------------------------------------
    
    void DrawCacheSettings() {
        GUIStyle styleFold = EditorStyles.foldout;
        styleFold.fontStyle = FontStyle.Bold;

        m_sceneCachePlayer.foldCacheSettings = EditorGUILayout.Foldout(m_sceneCachePlayer.foldCacheSettings, "Player", true, styleFold);
        if (m_sceneCachePlayer.foldCacheSettings) {
            //Show Selector GUI. Check if we should reopen
            string fullPath           = m_sceneCachePlayer.GetSceneCacheFilePath();
            string prevNormalizedPath = AssetUtility.NormalizeAssetPath(fullPath);

            string newNormalizedPath = EditorGUIDrawerUtility.DrawFileSelectorGUI("Cache File Path", "MeshSync", 
                prevNormalizedPath, "sc", OnSceneCacheFileReload);
            newNormalizedPath = AssetUtility.NormalizeAssetPath(newNormalizedPath);            

            if (newNormalizedPath != prevNormalizedPath) {
                ChangeSceneCacheFileInInspector(m_sceneCachePlayer, newNormalizedPath);
            }
            
            if (!string.IsNullOrEmpty(fullPath) && !fullPath.StartsWith(Application.streamingAssetsPath)) {
                GUILayout.BeginHorizontal();
                const float BUTTON_WIDTH = 50.0f;
                if (GUILayout.Button("Copy", GUILayout.Width(BUTTON_WIDTH))) {
                    string dstPath = Misc.CopyFileToStreamingAssets(fullPath);
                    ChangeSceneCacheFileInInspector(m_sceneCachePlayer, dstPath);
                }
                GUILayout.Label("Scene Cache file to StreamingAssets");
                EditorGUILayout.LabelField("(RECOMMENDED)", EditorStyles.boldLabel);
                GUILayout.FlexibleSpace();
                GUILayout.EndHorizontal();
                GUILayout.Space(15);
            }
            EditorGUILayout.Space();


            EditorGUI.BeginChangeCheck();
            
            SceneCachePlayer.TimeUnit selectedTimeUnit = (SceneCachePlayer.TimeUnit) 
                EditorGUILayout.Popup("Time Unit", (int)m_sceneCachePlayer.GetTimeUnit(), m_timeUnitEnums);
            if (EditorGUI.EndChangeCheck()) {
                m_sceneCachePlayer.SetTimeUnit(selectedTimeUnit);
                m_sceneCachePlayer.ResetTimeAnimation();
            }

            if (selectedTimeUnit == SceneCachePlayer.TimeUnit.Seconds) {
                m_sceneCachePlayer.SetTime(EditorGUILayout.FloatField("Time", m_sceneCachePlayer.GetTime()));
                m_sceneCachePlayer.SetInterpolation(EditorGUILayout.Toggle("Interpolation", m_sceneCachePlayer.GetInterpolation()));
            } else if (selectedTimeUnit == SceneCachePlayer.TimeUnit.Frames) {
                
                EditorGUI.BeginChangeCheck();

                SceneCachePlayer.BaseFrame selectedBaseFrame = (SceneCachePlayer.BaseFrame) 
                    EditorGUILayout.Popup("Base Frame", (int)m_sceneCachePlayer.GetBaseFrame(), m_baseFrameEnums);
                if (EditorGUI.EndChangeCheck()) {
                    m_sceneCachePlayer.SetBaseFrame(selectedBaseFrame);                    
                    m_sceneCachePlayer.ResetTimeAnimation();
                }

                m_sceneCachePlayer.SetFrame(EditorGUILayout.IntField("Frame", m_sceneCachePlayer.GetFrame()));
            }

            // preload
            {                
                int preloadLength = EditorGUILayout.IntSlider("Preload Length", m_sceneCachePlayer.GetPreloadLength(), 0, m_sceneCachePlayer.frameCount);
                m_sceneCachePlayer.SetPreloadLength(preloadLength);

            }

            EditorGUILayout.Space();
        }
    }

//----------------------------------------------------------------------------------------------------------------------
    void OnSceneCacheFileReload() {
        string sceneCacheFilePath = m_sceneCachePlayer.GetSceneCacheFilePath();
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(m_sceneCachePlayer, sceneCacheFilePath);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    private static void ChangeSceneCacheFileInInspector(SceneCachePlayer cachePlayer, string sceneCacheFilePath) {
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(cachePlayer, sceneCacheFilePath);
    }


//----------------------------------------------------------------------------------------------------------------------
    void ShowReloadSelectedSceneCacheFilesGUI() {
        float itemHeight = EditorGUIUtility.singleLineHeight;       
       
        //Button
        EditorGUILayout.BeginHorizontal(GUILayout.Height(itemHeight));            
        if (GUILayout.Button("Reload Selected Scene Cache Files", GUILayout.Width(250f),GUILayout.Height(itemHeight))) {            
            foreach (SceneCachePlayer player in m_targets) {
                string sceneCacheFilePath = player.GetSceneCacheFilePath();
                SceneCachePlayerEditorUtility.ChangeSceneCacheFile(player, sceneCacheFilePath);                
            }
        }
        EditorGUILayout.EndHorizontal();
        
        //[TODO-sin: 2020-9-28] use ScrollView
        foreach (SceneCachePlayer player in m_targets) {
            EditorGUILayout.BeginHorizontal(GUILayout.Height(itemHeight));            
            GUILayout.Space(30);            
            if (player.gameObject.IsPrefab()) {
                GameObject prefab = PrefabUtility.GetCorrespondingObjectFromOriginalSource(player.gameObject);                
                string prefabPath = AssetDatabase.GetAssetPath(prefab);
                EditorGUILayout.LabelField(prefabPath);                
            } else {
                EditorGUILayout.LabelField(player.name);                
            }
            EditorGUILayout.EndHorizontal();            
        }
        
    }
//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer       m_sceneCachePlayer = null;
    private List<SceneCachePlayer> m_targets          = new List<SceneCachePlayer>();


    private readonly string[] m_timeUnitEnums = System.Enum.GetNames( typeof( SceneCachePlayer.TimeUnit ) );
    private readonly string[] m_baseFrameEnums = System.Enum.GetNames( typeof( SceneCachePlayer.BaseFrame ) );
    

}

} //end namespace
