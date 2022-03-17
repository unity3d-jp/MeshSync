using System.Collections.Generic;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {

[CustomEditor(typeof(SceneCachePlayer))]
[CanEditMultipleObjects]
internal class SceneCachePlayerInspector : BaseMeshSyncInspector {
    

//----------------------------------------------------------------------------------------------------------------------

    public void OnEnable() {
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

        //prevent errors when applying new SceneCacheImporter settings
        if (null == m_sceneCachePlayer)
            return;

        
        EditorGUILayout.Space();
        bool changed = DrawCacheFile(m_sceneCachePlayer);
        DrawCacheInfo(m_sceneCachePlayer);
        changed |= DrawPlaybackMode(m_sceneCachePlayer);
        changed |= DrawAssetSyncSettings(m_sceneCachePlayer);
        changed |= DrawSceneCacheImportSettings(m_sceneCachePlayer);
        changed |= DrawMiscSettings(m_sceneCachePlayer);
        
        MeshSyncPlayerConfig config = m_sceneCachePlayer.GetConfigV();
        if (config.Profiling) {
            EditorGUILayout.TextArea(m_sceneCachePlayer.dbgProfileReport, GUILayout.Height(120));
            EditorGUILayout.Space();
        }

        changed |= DrawDefaultMaterialList(m_sceneCachePlayer);
        DrawExportAssets(m_sceneCachePlayer);
        DrawPluginVersion();

        if (!changed)
            return;
        
        PrefabUtility.RecordPrefabInstancePropertyModifications(m_sceneCachePlayer);
        
    }     

//----------------------------------------------------------------------------------------------------------------------
    
    bool DrawCacheFile(SceneCachePlayer t) {
        t.ShowCacheFileInInspector(EditorGUILayout.Foldout(t.IsCacheFileShownInInspector(), "File", true, GetDefaultFoldoutStyle()));
        if (!t.IsCacheFileShownInInspector()) 
            return false;
        
        bool changed = false;
        
        //Show Selector GUI. Check if we should reopen
        string fullPath           = t.GetSceneCacheFilePath();
        string prevNormalizedPath = AssetEditorUtility.NormalizePath(fullPath);

        string newNormalizedPath = EditorGUIDrawerUtility.DrawFileSelectorGUI("Cache File Path", "MeshSync", 
            prevNormalizedPath, "sc", OnSceneCacheFileReload);
        newNormalizedPath = AssetEditorUtility.NormalizePath(newNormalizedPath);            

        if (newNormalizedPath != prevNormalizedPath) {
            ChangeSceneCacheFileInInspector(t, newNormalizedPath);
        }
            
        if (!string.IsNullOrEmpty(fullPath) && !fullPath.StartsWith(Application.streamingAssetsPath)) {
            GUILayout.BeginHorizontal();
            const float BUTTON_WIDTH = 50.0f;
            if (GUILayout.Button("Copy", GUILayout.Width(BUTTON_WIDTH))) {
                string dstPath = Misc.CopyFileToStreamingAssets(fullPath);
                ChangeSceneCacheFileInInspector(t, dstPath);
            }
            GUILayout.Label("Scene Cache file to StreamingAssets");
            EditorGUILayout.LabelField("(RECOMMENDED)", EditorStyles.boldLabel);
            GUILayout.FlexibleSpace();
            GUILayout.EndHorizontal();
            GUILayout.Space(15);
        }
        EditorGUILayout.Space();

        //[TODO-sin: 2022-3-14] This may cause crash when sliding the values back and forth. Find out why
        // preload
        {                
            // changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "SceneCache: Preload",
            //     guiFunc: () => (EditorGUILayout.IntSlider("Preload Length", t.GetPreloadLength(), 0, t.frameCount)),
            //     updateFunc: (int preloadLength) => { t.SetPreloadLength(preloadLength); });
        }

        return changed;
    }

    private static void DrawCacheInfo(SceneCachePlayer t) {

        ISceneCacheInfo scInfo = t.ExtractSceneCacheInfo(forceOpen:false);
        if (null == scInfo)
            return;
        
        t.ShowInfoInInspector(EditorGUILayout.Foldout(t.IsInfoInInspectorShown(), "Info", true, GetDefaultFoldoutStyle()));
        if (!t.IsInfoInInspectorShown()) 
            return;
        
        ++EditorGUI.indentLevel;

        const int leftWidth  = 160;
        int       numFrames  = scInfo.GetNumFrames();
        float     sampleRate = scInfo.GetSampleRate();
        TimeRange timeRange  = scInfo.GetTimeRange();

        EditorGUILayout.BeginHorizontal();
        GUILayout.Label($"Num Frames: {numFrames}",GUILayout.MaxWidth(leftWidth));
        GUILayout.Label($"Start Time: {timeRange.start,15:N4}");
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        GUILayout.Label($"Frame Rate: {sampleRate}",GUILayout.MaxWidth(leftWidth));
        GUILayout.Label($"End Time  : {timeRange.end,15:N4}");
        EditorGUILayout.EndHorizontal();
               
        --EditorGUI.indentLevel;
        
        EditorGUILayout.Space();
        EditorGUILayout.Space();
    }

//----------------------------------------------------------------------------------------------------------------------
    private static bool DrawPlaybackMode(SceneCachePlayer t) {

        t.ShowPlaybackInInspector(EditorGUILayout.Foldout(t.IsPlaybackInInspectorShown(), "Playback", true, GetDefaultFoldoutStyle()));
        if (!t.IsPlaybackInInspectorShown()) 
            return false;
        
        bool changed = false;
        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t,"SceneCache: Playback Mode",
            guiFunc: () => 
                (SceneCachePlaybackMode)EditorGUILayout.EnumPopup("Playback Mode", t.GetPlaybackMode()), 
            updateFunc: (SceneCachePlaybackMode mode) => {
                t.SetPlaybackMode(mode);
                SceneCachePlayerEditorUtility.RefreshSceneCache(t);
            }
        );

        ++EditorGUI.indentLevel;
        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "SceneCache: Time",
            guiFunc: () => (EditorGUILayout.FloatField("Time", t.GetTime())),
            updateFunc: (float time) => { t.SetTime(Mathf.Max(0,time)); });
            
        using (new EditorGUI.DisabledScope(t.GetPlaybackMode() == SceneCachePlaybackMode.Interpolate)) {
            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "SceneCache: Frame",
                guiFunc: () => (EditorGUILayout.IntField("Frame", t.GetFrame())),
                updateFunc: (int frame) => { t.SetTimeByFrame(Mathf.Max(0,frame)); });
        }
        --EditorGUI.indentLevel;

        using (new EditorGUI.DisabledScope(t.GetPlaybackMode() == SceneCachePlaybackMode.Interpolate)) {
            changed |= SceneCachePlayerEditorUtility.DrawLimitedAnimationGUI(t.GetLimitedAnimationController(), t, t);
        }

        EditorGUILayout.Space();

        return changed;
    }
    
   

//----------------------------------------------------------------------------------------------------------------------

    private static bool DrawSceneCacheImportSettings(SceneCachePlayer t) {

        MeshSyncPlayerConfig playerConfig = t.GetConfigV();
        
        t.foldImportSettings = EditorGUILayout.Foldout(t.foldImportSettings, "Import Settings", true, GetBoldFoldoutStyle());
        if (!t.foldImportSettings) 
            return false;

        bool changed = false;
        
        IHasModelImporterSettings importer = AssetImporter.GetAtPath(t.GetSceneCacheFilePath()) as IHasModelImporterSettings;
        ModelImporterSettings importerSettings = playerConfig.GetModelImporterSettings();
                
        if (null == importer) {
            MeshSyncInspectorUtility.DrawModelImporterSettingsGUI(t, importerSettings);
        } else {

            bool isOverride = t.IsModelImporterSettingsOverridden();
                
            EditorGUILayout.BeginHorizontal();
            EditorGUIDrawerUtility.DrawUndoableGUI(t, "Override",
                guiFunc: () => GUILayout.Toggle(isOverride, "", GUILayout.MaxWidth(15.0f)), 
                updateFunc: (bool overrideValue) => { t.OverrideModelImporterSettings(overrideValue); });

            using (new EditorGUI.DisabledScope(!isOverride)) {
                EditorGUIDrawerUtility.DrawUndoableGUI(t, "Create Materials",
                    guiFunc: () => (bool)EditorGUILayout.Toggle("Create Materials", importerSettings.CreateMaterials),
                    updateFunc: (bool createMat) => { importerSettings.CreateMaterials = createMat; });
            }
                
            EditorGUILayout.EndHorizontal();

            using (new EditorGUI.DisabledScope(!isOverride)) {
                ++EditorGUI.indentLevel;
                MeshSyncInspectorUtility.DrawModelImporterMaterialSearchMode(t, importerSettings);
                --EditorGUI.indentLevel;
            }
                
        }
            
            

        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Animation Interpolation",
            guiFunc: () => EditorGUILayout.Popup(new GUIContent("Animation Interpolation"),
                playerConfig.AnimationInterpolation, MeshSyncEditorConstants.ANIMATION_INTERPOLATION_ENUMS),
            updateFunc: (int val) => { playerConfig.AnimationInterpolation = val; }
        );


        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Keyframe Reduction",
            guiFunc: () => EditorGUILayout.Toggle("Keyframe Reduction", playerConfig.KeyframeReduction),
            updateFunc: (bool toggle) => { playerConfig.KeyframeReduction = toggle; }
        );

        if (playerConfig.KeyframeReduction) {
            EditorGUI.indentLevel++;

            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Threshold",
                guiFunc: () => EditorGUILayout.FloatField("Threshold", playerConfig.ReductionThreshold),
                updateFunc: (float val) => { playerConfig.ReductionThreshold = val; }
            );

            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Erase Flat Curves",
                guiFunc: () => EditorGUILayout.Toggle("Erase Flat Curves", playerConfig.ReductionEraseFlatCurves),
                updateFunc: (bool toggle) => { playerConfig.ReductionEraseFlatCurves = toggle; }
            );
            EditorGUI.indentLevel--;
        }

        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Z-Up Correction",
            guiFunc: () => EditorGUILayout.Popup(new GUIContent("Z-Up Correction"), playerConfig.ZUpCorrection,
                MeshSyncEditorConstants.Z_UP_CORRECTION_ENUMS),
            updateFunc: (int val) => { playerConfig.ZUpCorrection = val; }
        );

        EditorGUILayout.Space();

        return changed;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    void OnSceneCacheFileReload() {
        SceneCachePlayerEditorUtility.ReloadSceneCacheFile(m_sceneCachePlayer);
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
    
    static GUIStyle GetDefaultFoldoutStyle() {
        GUIStyle foldStyle = EditorStyles.foldout;
        foldStyle.fontStyle = FontStyle.Bold;
        return foldStyle;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer       m_sceneCachePlayer = null;
    private List<SceneCachePlayer> m_targets          = new List<SceneCachePlayer>();


    private static readonly string[] TIME_UNIT_ENUMS = System.Enum.GetNames( typeof( SceneCachePlayer.TimeUnit ) );
    

}

} //end namespace
