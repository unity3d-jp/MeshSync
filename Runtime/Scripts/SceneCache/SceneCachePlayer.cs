using System;
using System.ComponentModel;
using System.Text;
using JetBrains.Annotations;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Serialization;

#if UNITY_EDITOR
using UnityEditor;
using Unity.FilmInternalUtilities.Editor;
#endif

namespace Unity.MeshSync {
/// <summary>
/// SceneCachePlayer controls the playback of an .sc file that has been exported
/// using MeshSyncDCCPlugins installed in a DCC Tool.
/// </summary>
[ExecuteInEditMode]
public class SceneCachePlayer : BaseMeshSync {
    #region Types

    internal enum TimeUnit {
        Seconds,
        Frames
    }

    #endregion


//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer() : base() {
        SetSaveAssetsInScene(false);
        MarkMeshesDynamic(true);
    }

    private protected override void InitInternalV() {
    }

    private protected override void UpdateMaterialAssetV(MaterialData materialData) {
        ModelImporterSettings modelImporterSettings = m_config.GetModelImporterSettings();


#if UNITY_EDITOR
        //Get the settings from the SceneCacheImporter if not set to override
        if (AssetImporter.GetAtPath(m_sceneCacheFilePath) is IHasModelImporterSettings importer
            && !m_overrideModelImporterSettings)
            modelImporterSettings = importer.GetModelImporterSettings();
#endif

        UpdateMaterialAssetByDefault(materialData,  modelImporterSettings);
    }

//----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Force SceneCache to be updated and loaded to scene.
    /// </summary>
    public void ForceUpdate() {
        LoadSceneCacheToScene(m_time, false);
    }

//----------------------------------------------------------------------------------------------------------------------

    internal string GetSceneCacheFilePath() {
        return m_sceneCacheFilePath;
    }

    internal bool IsSceneCacheOpened() {
        return m_sceneCache;
    }

    internal override MeshSyncPlayerConfig GetConfigV() {
        return m_config;
    }

    internal SceneCachePlaybackMode GetPlaybackMode() {
        return m_playbackMode;
    }

    internal void SetPlaybackMode(SceneCachePlaybackMode mode) {
        m_playbackMode = mode;
    }

    internal float GetTime() {
        return m_time;
    }

    internal void SetTime(float time) {
        m_time = time;
    }

    internal int GetFrame() {
        return m_frame;
    }

    internal void SetTimeByFrame(int frame) {
        if (!m_sceneCache)
            return;

        frame  = Mathf.Clamp(frame, 0, m_sceneCache.GetNumScenes());
        m_time = (float)frame / m_sceneCache.GetSampleRate();
    }

    internal int GetPreloadLength() {
        return m_preloadLength;
    }

    internal void SetPreloadLength(int preloadLength) {
        m_preloadLength = preloadLength;
    }

    internal bool IsModelImporterSettingsOverridden() {
        return m_overrideModelImporterSettings;
    }

    internal void OverrideModelImporterSettings(bool overrideValue) {
        m_overrideModelImporterSettings = overrideValue;
    }

//----------------------------------------------------------------------------------------------------------------------

    [CanBeNull]
    internal SceneCacheInfo ExtractSceneCacheInfo(bool forceOpen) {
        if (IsSceneCacheOpened()) return m_sceneCacheInfo;

        if (!forceOpen)
            return null;

        SceneCacheData tempSceneCache = SceneCacheData.Open(m_sceneCacheFilePath);
        if (!tempSceneCache) return null;

        SceneCacheInfo ret = new SceneCacheInfo();
        UpdateSceneCacheInfo(ret, tempSceneCache);
        tempSceneCache.Close();

        return ret;
    }

//----------------------------------------------------------------------------------------------------------------------

    #region Properties

#if UNITY_EDITOR

    internal bool IsCacheFileShownInInspector() {
        return m_showCacheFileInInspector;
    }

    internal void ShowCacheFileInInspector(bool show) {
        m_showCacheFileInInspector = show;
    }

    internal bool IsPlaybackInInspectorShown() {
        return m_showPlaybackInInspector;
    }

    internal void ShowPlaybackInInspector(bool show) {
        m_showPlaybackInInspector = show;
    }

    internal bool IsInfoInInspectorShown() {
        return m_showInfoInInspector;
    }

    internal void ShowInfoInInspector(bool show) {
        m_showInfoInInspector = show;
    }


    internal string dbgProfileReport {
        get { return m_dbgProfileReport; }
    }
#endif

    #endregion

    #region Internal Methods

#if UNITY_EDITOR
    internal bool OpenCacheInEditor(string path) {
        string normalizedPath = System.IO.Path.GetFullPath(path).Replace('\\', '/');
        normalizedPath = AssetEditorUtility.NormalizePath(normalizedPath);

        if (!OpenCacheInternal(normalizedPath, true)) return false;

        ExportMaterials(false, true);

        if (m_sceneCache) {
            SceneData scene = LoadSceneData(m_loadedTime, out _);
            //[TODO-sin: 2022-3-9] Review if this code is necessary.
            //Was added in commit  b60337aff38e55febf81a9b7c741458eff34a919 on August 18, 2019.
            if (scene && !scene.submeshesHaveUniqueMaterial) m_config.SyncMaterialList = false;
        }

        return true;
    }

    private void SavePrefabInEditor() {
        PrefabUtility.RecordPrefabInstancePropertyModifications(this);
    }

#endif //UNITY_EDITOR    

//----------------------------------------------------------------------------------------------------------------------    

    private bool OpenCacheInternal(string path, bool updateNonMaterialAssets) {
        CloseCache();

        m_sceneCache = SceneCacheData.Open(path);
        if (!m_sceneCache) {
            Debug.LogError($"SceneCachePlayer: cache open failed ({path})");
            return false;
        }

        m_sceneCacheFilePath = path;

        UpdateSceneCacheInfo(m_sceneCacheInfo, m_sceneCache);

#if UNITY_EDITOR
        SetSortEntities(true);
#endif
        LogDebug($"SceneCachePlayer: cache opened ({path})");

        //[Note-sin: 2021-7-19] Time/Frame 0 must be loaded first, because the data of other frames might contain "No change from frame 0" 
        LoadSceneCacheToScene(0, updateNonMaterialAssets);

        return true;
    }

    private static void UpdateSceneCacheInfo(SceneCacheInfo scInfo, SceneCacheData scData) {
        Assert.IsTrue(scData);

        scInfo.sampleRate = scData.GetSampleRate();
        scInfo.timeCurve  = scData.GetTimeCurve(InterpolationMode.Constant);
        scInfo.timeRange  = scData.GetTimeRange();

        float duration = scInfo.timeRange.GetDuration();
        scInfo.numFrames = Mathf.Min((int)Mathf.Floor(duration * scInfo.sampleRate), scData.GetNumScenes());
    }

    internal void CloseCache() {
        if (m_sceneCache) {
            m_sceneCache.Close();
            m_sceneCacheInfo.Reset();
            LogDebug($"SceneCachePlayer: cache closed ({m_sceneCacheFilePath})");
        }

        m_loadedTime = -1;
    }


//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    private void UpdatePlayer(bool updateNonMaterialAssets) {
        if (!m_sceneCache) return;

        if (m_time != m_loadedTime) {
            LoadSceneCacheToScene(m_time, updateNonMaterialAssets);
        }
        else if (m_sceneCache.GetPreloadLength() != m_preloadLength) {
            m_sceneCache.SetPreloadLength(m_preloadLength);
            m_sceneCache.Preload(m_sceneCache.GetFrame(m_time));
        }
    }

    #endregion


    private void LoadSceneCacheToScene(float time, bool updateNonMaterialAssets) {
        m_sceneCache.SetPreloadLength(m_preloadLength);
#if UNITY_EDITOR
        ulong sceneGetBegin = Misc.GetTimeNS();
#endif

        SceneData scene = LoadSceneData(time, out m_frame);
        m_loadedTime = time;

        // get scene
#if UNITY_EDITOR
        m_dbgSceneGetTime = Misc.NS2MS(Misc.GetTimeNS() - sceneGetBegin);
#endif

        if (scene) {
#if UNITY_EDITOR
            ulong sceneUpdateBegin = Misc.GetTimeNS();
#endif

            // update scene
            BeforeUpdateScene();
            UpdateScene(scene, updateNonMaterialAssets, false);
            AfterUpdateScene();
#if UNITY_EDITOR
            SetSortEntities(false);

            if (m_config.Profiling) {
                m_dbgSceneUpdateTime = Misc.NS2MS(Misc.GetTimeNS() - sceneUpdateBegin);
                UpdateProfileReport(scene);
            }
#endif
        }
    }

    //frame: the frame that corresponds to the time parameter, if applicable
    private SceneData LoadSceneData(float time, out int frame) {
        SceneData scene = default;
        frame = m_frame; //no change by default        
        switch (m_playbackMode) {
            case SceneCachePlaybackMode.SnapToPreviousFrame: {
                frame = CalculateFrameByFloor(time, m_sceneCacheInfo);
                scene = m_sceneCache.LoadByFrame(frame);
                break;
            }

            case SceneCachePlaybackMode.SnapToNearestFrame: {
                frame = CalculateFrameByRound(time, m_sceneCacheInfo);
                scene = m_sceneCache.LoadByFrame(frame);
                break;
            }
            case SceneCachePlaybackMode.Interpolate: {
                scene = m_sceneCache.LoadByTime(m_time, true);
                break;
            }
        }

        return scene;
    }

    internal int CalculateFrame(float time, LimitedAnimationController limitedAnimationController) {
        int frame = 0;
        switch (m_playbackMode) {
            case SceneCachePlaybackMode.SnapToPreviousFrame: {
                frame = CalculateFrameByFloor(time, m_sceneCacheInfo, limitedAnimationController);
                break;
            }

            case SceneCachePlaybackMode.SnapToNearestFrame: {
                frame = CalculateFrameByRound(time, m_sceneCacheInfo, limitedAnimationController);
                break;
            }
            default: {
                Assert.IsTrue(false); //invalid call
                break;
            }
        }

        return frame;
    }

    internal static float CalculatePlaybackTime(float time, SceneCachePlaybackMode playbackMode, SceneCacheInfo scInfo) {
        switch (playbackMode) {
            case SceneCachePlaybackMode.SnapToPreviousFrame: {
                int frame = CalculateFrameByFloor(time, scInfo);
                return frame / scInfo.sampleRate;
            }

            case SceneCachePlaybackMode.SnapToNearestFrame: {
                int frame = CalculateFrameByRound(time, scInfo);
                return frame / scInfo.sampleRate;
            }
            default: {
                return time;
            }
        }
    }


    private static int CalculateFrameByFloor(float time, SceneCacheInfo scInfo, LimitedAnimationController controller) {
        int frame = Mathf.FloorToInt(time * scInfo.sampleRate);
        frame = controller.Apply(frame);
        frame = Mathf.Clamp(frame, 0, scInfo.numFrames - 1);
        return frame;
    }

    private static int CalculateFrameByRound(float time, SceneCacheInfo scInfo, LimitedAnimationController controller) {
        int frame = Mathf.RoundToInt(time * scInfo.sampleRate);
        frame = controller.Apply(frame);
        frame = Mathf.Clamp(frame, 0, scInfo.numFrames - 1);
        return frame;
    }

    private static int CalculateFrameByFloor(float time, SceneCacheInfo scInfo) {
        int frame = Mathf.FloorToInt(time * scInfo.sampleRate);
        frame = Mathf.Clamp(frame, 0, scInfo.numFrames - 1);
        return frame;
    }

    private static int CalculateFrameByRound(float time, SceneCacheInfo scInfo) {
        int frame = Mathf.RoundToInt(time * scInfo.sampleRate);
        frame = Mathf.Clamp(frame, 0, scInfo.numFrames - 1);
        return frame;
    }

//----------------------------------------------------------------------------------------------------------------------
    internal bool IsLimitedAnimationOverrideable() {
        if (m_playbackMode == SceneCachePlaybackMode.Interpolate)
            return false;

        return true;
    }

    internal void AllowLimitedAnimationOverride() {
        if (m_playbackMode == SceneCachePlaybackMode.Interpolate)
            m_playbackMode = SceneCachePlaybackMode.SnapToNearestFrame;
    }

//----------------------------------------------------------------------------------------------------------------------

    private protected override void OnBeforeSerializeMeshSyncPlayerV() {
        m_sceneCachePlayerVersion = CUR_SCENE_CACHE_PLAYER_VERSION;
    }

    private protected override void OnAfterDeserializeMeshSyncPlayerV() {
        if (m_sceneCachePlayerVersion == CUR_SCENE_CACHE_PLAYER_VERSION)
            return;

#if UNITY_EDITOR
        if (m_sceneCachePlayerVersion < (int)SceneCachePlayerVersion.NormalizedPath_0_9_2)
            m_sceneCacheFilePath = AssetEditorUtility.NormalizePath(m_sceneCacheFilePath);
#pragma warning disable 612
        if (m_sceneCachePlayerVersion < (int)SceneCachePlayerVersion.PlaybackMode_0_12_0
            && m_timeUnit == TimeUnit.Frames)
            m_timeUnit = TimeUnit.Seconds;
#pragma warning restore 612

        if (m_sceneCachePlayerVersion < (int)SceneCachePlayerVersion.RemoveAnimator_0_16_0)
            Debug.LogWarning("[MeshSync] SceneCachePlayer no longer requires Animator component. Please remove it if applicable.");
#endif

        m_sceneCachePlayerVersion = CUR_SCENE_CACHE_PLAYER_VERSION;
    }

//----------------------------------------------------------------------------------------------------------------------


#if UNITY_EDITOR
    private void UpdateProfileReport(SceneData data) {
        StringBuilder    sb   = new StringBuilder();
        SceneProfileData prof = data.profileData;
        sb.AppendFormat("Scene Get: {0:#.##}ms\n", m_dbgSceneGetTime);
        sb.AppendFormat("Scene Update: {0:#.##}ms\n", m_dbgSceneUpdateTime);
        sb.AppendFormat("\n");

        {
            ulong sizeEncoded = prof.sizeEncoded;
            if (sizeEncoded > 1000000)
                sb.AppendFormat("Cache: {0:#.##}MB encoded, {1:#.##}MB decoded, ", sizeEncoded / 1000000.0, prof.sizeDecoded / 1000000.0);
            else if (sizeEncoded > 1000)
                sb.AppendFormat("Cache: {0:#.##}KB encoded, {1:#.##}KB decoded, ", sizeEncoded / 1000.0, prof.sizeDecoded / 1000.0);
            else
                sb.AppendFormat("Cache: {0}B encoded, {1}B decoded, ", sizeEncoded, prof.sizeDecoded);
            sb.AppendFormat("{0} verts\n", prof.vertexCount);
        }
        sb.AppendFormat("Cache Load: {0:#.##}ms\n", prof.loadTime);
        double MBperSec = (double)prof.sizeEncoded / 1000000.0 / (prof.readTime / 1000.0);
        sb.AppendFormat("  Cache Read: {0:#.##}ms ({1:#.##}MB/sec)\n", prof.readTime, MBperSec);
        sb.AppendFormat("  Cache Decode: {0:#.##}ms (total of worker threads)\n", prof.decodeTime);
        if (prof.setupTime > 0)
            sb.AppendFormat("Setup Scene: {0:#.##}ms\n", prof.setupTime);
        if (prof.lerpTime > 0)
            sb.AppendFormat("Interpolate Scene: {0:#.##}ms\n", prof.lerpTime);
        m_dbgProfileReport = sb.ToString();
    }


#endif //UNITY_EDITOR

//----------------------------------------------------------------------------------------------------------------------

    private void LogDebug(string logMessage) {
        if (!m_config.Logging)
            return;

        Debug.Log(logMessage);
    }

//----------------------------------------------------------------------------------------------------------------------

    private float ClampTime(float time) {
        return Mathf.Clamp(time, m_sceneCacheInfo.timeRange.start, m_sceneCacheInfo.timeRange.end);
    }

//----------------------------------------------------------------------------------------------------------------------

    #region Events

#if UNITY_EDITOR
    private void Reset() {
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateInstance();
        m_config = new SceneCachePlayerConfig(projectSettings.GetDefaultSceneCachePlayerConfig());
    }

    private void OnValidate() {
        if (!m_sceneCache)
            return;

        m_time = ClampTime(m_time);
    }
#endif

//----------------------------------------------------------------------------------------------------------------------    
    private protected override void OnEnable() {
        base.OnEnable();

#if UNITY_EDITOR
        m_onMaterialChangedInSceneViewCB += SavePrefabInEditor;
#endif

        if (!string.IsNullOrEmpty(m_sceneCacheFilePath)) OpenCacheInternal(m_sceneCacheFilePath, false);

        if (!m_sceneCache)
            return;

        m_time = ClampTime(m_time);
    }

    private protected override void OnDisable() {
        base.OnDisable();
#if UNITY_EDITOR
        m_onMaterialChangedInSceneViewCB -= SavePrefabInEditor;
#endif

        CloseCache();
    }

    // note:
    // Update() is called *before* animation update.
    // in many cases m_time is controlled by animation system. so scene update must be handled in LateUpdate()
    private void LateUpdate() {
        UpdatePlayer(false);
    }

    #endregion //Events

//----------------------------------------------------------------------------------------------------------------------

    [SerializeField] private string m_sceneCacheFilePath = null; //The full path of the file. Use '/'

    [Obsolete] [SerializeField] private TimeUnit m_timeUnit = TimeUnit.Seconds;

    [SerializeField] private SceneCachePlaybackMode m_playbackMode = SceneCachePlaybackMode.SnapToNearestFrame;

    [SerializeField] private float m_time;
    [SerializeField] private int   m_preloadLength = 1;

    [SerializeField] private SceneCachePlayerConfig m_config;

    //Foldout settings
    [SerializeField] private bool m_showCacheFileInInspector = true;
    [SerializeField] private bool m_showPlaybackInInspector  = true;
    [SerializeField] private bool m_showInfoInInspector      = false;

    //only used when the sceneCacheFilePath has a valid importer (under Assets)
    [SerializeField] private bool m_overrideModelImporterSettings = false;

    //Renamed in 0.10.x-preview
    [FormerlySerializedAs("m_version")] [HideInInspector] [SerializeField]
    private int m_sceneCachePlayerVersion = (int)SceneCachePlayerVersion.NoVersioning;

    private const int CUR_SCENE_CACHE_PLAYER_VERSION = (int)SceneCachePlayerVersion.RemoveAnimator_0_16_0;

    private SceneCacheData m_sceneCache;

    private readonly SceneCacheInfo m_sceneCacheInfo = new SceneCacheInfo();

    private int   m_frame      = 0;
    private float m_loadedTime = -1;

#if UNITY_EDITOR
    private float  m_dbgSceneGetTime;
    private float  m_dbgSceneUpdateTime;
    private string m_dbgProfileReport;
#endif

//----------------------------------------------------------------------------------------------------------------------    

    private enum SceneCachePlayerVersion {
        NoVersioning          = 0, //Didn't have versioning in earlier versions
        StringPath_0_4_0      = 2, //0.4.0-preview: the path is declared as a string 
        NormalizedPath_0_9_2  = 3, //0.9.2-preview: Path must be normalized by default 
        PlaybackMode_0_12_0   = 4, //0.12.0-preview: integrate frame/time unit and interpolation into playback mode  
        RemoveAnimator_0_16_0 = 5  //0.16.0-preview. Animator is removed, and animating has to be performed via Timeline  
    }
}
} //end namespace