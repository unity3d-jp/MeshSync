using System;
using System.Text;
using JetBrains.Annotations;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Serialization;

#if UNITY_EDITOR
using UnityEditor;
using Unity.FilmInternalUtilities.Editor;
#endif

namespace Unity.MeshSync
{


/// <summary>
/// SceneCachePlayer controls the playback of an .sc file that has been exported
/// using MeshSyncDCCPlugins installed in a DCC Tool.
/// </summary>
[RequireComponent(typeof(Animator))]
[ExecuteInEditMode]
public class SceneCachePlayer : BaseMeshSync {
    #region Types
    internal enum TimeUnit {
        Seconds,
        Frames,
    }    
    #endregion


//----------------------------------------------------------------------------------------------------------------------
    
    SceneCachePlayer() : base() {
        SetSaveAssetsInScene(false);
        MarkMeshesDynamic(true);
    }

    protected override void InitInternalV() {
        
    }

    private protected override void UpdateMaterialAssetV(MaterialData materialData) {
        
        ModelImporterSettings modelImporterSettings = m_config.GetModelImporterSettings();

        
#if UNITY_EDITOR
        //Get the settings from the SceneCacheImporter if not set to override
        if (AssetImporter.GetAtPath(m_sceneCacheFilePath) is IHasModelImporterSettings importer 
            && !m_overrideModelImporterSettings) 
        {
            modelImporterSettings = importer.GetModelImporterSettings();
        } 
#endif            
        
        UpdateMaterialAssetByDefault(materialData,  modelImporterSettings);
    }
    
//----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Force SceneCache to be updated and loaded to scene.
    /// </summary>
    public void ForceUpdate() {
        UpdatePlayer( updateNonMaterialAssets: false);        
    }
    
//----------------------------------------------------------------------------------------------------------------------

    internal string GetSceneCacheFilePath() { return m_sceneCacheFilePath; }
    internal bool IsSceneCacheOpened() { return m_sceneCache;}

    internal override MeshSyncPlayerConfig GetConfigV() => m_config;
    
    internal void SetAutoplay(bool autoPlay) {
        //[Note-sin: 2021-1-18] May be called before m_animator is initialized in Playmode.
        //It is expected that the animator was already disabled previously in EditMode though.
        if (null == m_animator)
            return;
        
        m_animator.enabled = autoPlay;
    }

    internal float GetRequestedNormalizedTime() { return m_reqNormalizedTime;}

    internal TimeUnit GetTimeUnit() { return m_timeUnit; }

    internal void SetTimeUnit(TimeUnit timeUnit) {
        m_timeUnit = timeUnit;
    }

    internal SceneCachePlaybackMode GetPlaybackMode()                             { return m_playbackMode; }
    internal void                   SetPlaybackMode(SceneCachePlaybackMode mode ) { m_playbackMode = mode; }

    internal float GetTime() { return m_time;}
    internal void SetTime(float time) { m_time = time; }

    internal int GetFrame() { return m_frame; }
    internal void SetFrame(int frame) { m_frame = frame;}

    internal int GetPreloadLength() { return m_preloadLength;}
    internal void SetPreloadLength(int preloadLength) { m_preloadLength = preloadLength;}

    internal bool IsModelImporterSettingsOverridden() => m_overrideModelImporterSettings;

    internal void OverrideModelImporterSettings(bool overrideValue) {
        m_overrideModelImporterSettings = overrideValue;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    internal void RequestNormalizedTime(float normalizedTime) {
        m_reqNormalizedTime = normalizedTime;
        float time = normalizedTime * m_timeRange.end;
        
        switch (m_timeUnit) {
            case TimeUnit.Seconds: {
                m_time = time;
                ClampTime();
                break;
            }
            case TimeUnit.Frames: {
                m_frame = m_sceneCache.GetFrame(time);                
                break;
            }
            default: break;
        }
       
        
    }

    [CanBeNull]
    internal AnimationCurve GetTimeCurve() {
        if (!IsSceneCacheOpened())
            return null;
        
        return m_sceneCache.GetTimeCurve(InterpolationMode.Constant);
    }

    internal TimeRange GetTimeRange() { return m_timeRange;}
    
    

//----------------------------------------------------------------------------------------------------------------------
    #region Properties
    internal int frameCount {
        get { return m_sceneCache.sceneCount; }
    }

#if UNITY_EDITOR
    internal bool foldCacheSettings {
        get { return m_foldCacheSettings; }
        set { m_foldCacheSettings = value; }
    }
    internal string dbgProfileReport {
        get { return m_dbgProfileReport; }
    }
#endif
    #endregion

    #region Internal Methods

#if UNITY_EDITOR    
    internal bool OpenCacheInEditor(string path) {
        
        string normalizedPath = System.IO.Path.GetFullPath(path).Replace('\\','/');
        normalizedPath = AssetEditorUtility.NormalizePath(normalizedPath);

        if (!OpenCacheInternal(normalizedPath)) {
            return false;
        }
        
        UpdatePlayer(/* updateNonMaterialAssets = */ true);
        ExportMaterials(false, true);
        ResetTimeAnimation();

        if (m_sceneCache) {
            SceneData scene = LoadSceneData(m_timePrev);
            //[TODO-sin: 2022-3-9] Review if this code is necessary.
            //Was added in commit  b60337aff38e55febf81a9b7c741458eff34a919 on August 18, 2019.
            if (!scene.submeshesHaveUniqueMaterial) {
                m_config.SyncMaterialList = false;
            }
        }
        
        return true;
    }   

    void SavePrefabInEditor() {
        PrefabUtility.RecordPrefabInstancePropertyModifications(this);
    }
    
#endif //UNITY_EDITOR    

//----------------------------------------------------------------------------------------------------------------------    

    private bool OpenCacheInternal(string path) {
        CloseCache();

        m_sceneCache = SceneCacheData.Open(path);
        if (!m_sceneCache) {
            Debug.LogError($"SceneCachePlayer: cache open failed ({path})");
            return false;            
        }

        m_sceneCacheFilePath = path;
        m_timeRange= m_sceneCache.timeRange;
        
#if UNITY_EDITOR
        SetSortEntities(true);
#endif
        LogDebug($"SceneCachePlayer: cache opened ({path})");

        //[Note-sin: 2021-7-19] Time/Frame 0 must be loaded first, because the data of other frames might contain "No change from frame 0" 
        LoadSceneCacheToScene(0, updateNonMaterialAssets: false);
        
        return true;
    }
    
    internal void CloseCache() {
        if (m_sceneCache) {
            m_sceneCache.Close();
            LogDebug($"SceneCachePlayer: cache closed ({m_sceneCacheFilePath})");
        }
        m_timePrev = -1;
    }

    
//----------------------------------------------------------------------------------------------------------------------
    
#if UNITY_EDITOR

    private RuntimeAnimatorController GetOrCreateAnimatorControllerWithClip() {

        //paths
        string assetsFolder   = GetAssetsFolder();
        string goName         = gameObject.name;
        string animPath       = $"{assetsFolder}/{goName}.anim";
        string controllerPath = $"{assetsFolder}/{goName}.controller";

        //reuse
        if (null == m_animator.runtimeAnimatorController) { 
            m_animator.runtimeAnimatorController = AssetDatabase.LoadAssetAtPath<RuntimeAnimatorController>(controllerPath);
        } 
        
        RuntimeAnimatorController animatorController = m_animator.runtimeAnimatorController; 
        if (animatorController != null) {
            AnimationClip[] clips = animatorController.animationClips;
            if (clips != null && clips.Length > 0) {
                AnimationClip tmp = animatorController.animationClips[0];
                if (tmp != null) {
                    return animatorController;
                }
            }
        }
   
        
        AnimationClip clip = new AnimationClip();
        Misc.OverwriteOrCreateAsset(clip, animPath);
        Assert.IsNotNull(clip);

        animatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(controllerPath, clip);        
        m_animator.runtimeAnimatorController = animatorController; 

        return animatorController;
    }
    
    
    internal bool ResetTimeAnimation() {
        if (m_sceneCache.sceneCount < 2)
            return false;

        RuntimeAnimatorController animatorController = GetOrCreateAnimatorControllerWithClip();
        Assert.IsNotNull(animatorController);
        Assert.IsNotNull(animatorController.animationClips);
        Assert.IsTrue(animatorController.animationClips.Length > 0);
        AnimationClip clip = animatorController.animationClips[0];
        Assert.IsNotNull(clip);

        Undo.RegisterCompleteObjectUndo(clip, "SceneCachePlayer");
        
        float sampleRate = m_sceneCache.sampleRate;
        if (sampleRate > 0.0f)
            clip.frameRate = sampleRate;

        Type tPlayer = typeof(SceneCachePlayer);
        clip.SetCurve("", tPlayer, "m_time", null);
        clip.SetCurve("", tPlayer, "m_frame", null);
        if (m_timeUnit == TimeUnit.Seconds) {
            AnimationCurve curve = m_sceneCache.GetTimeCurve(InterpolationMode.Constant);
            clip.SetCurve("", tPlayer, "m_time", curve);
        } else if (m_timeUnit == TimeUnit.Frames) {
            AnimationCurve curve = m_sceneCache.GetFrameCurve();
            clip.SetCurve("", tPlayer, "m_frame", curve);
        }
        

        AssetDatabase.SaveAssets();
        UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        return true;
    }
#endif

    private void UpdatePlayer(bool updateNonMaterialAssets) {


        if (m_timeUnit == TimeUnit.Frames) {
            m_frame = Mathf.Clamp(m_frame, 0, frameCount);
            m_time = m_sceneCache.GetTime(m_frame);
        }

        if (!m_sceneCache) {
            return;
        }
        
        if (m_time != m_timePrev) {
            LoadSceneCacheToScene(m_time, updateNonMaterialAssets);
        } else if(m_sceneCache.preloadLength != m_preloadLength) {
            m_sceneCache.preloadLength = m_preloadLength;
            m_sceneCache.Preload(m_sceneCache.GetFrame(m_time));
        }

    }
    #endregion


    void LoadSceneCacheToScene(float time, bool updateNonMaterialAssets) {
        m_timePrev = m_time = time;
        m_sceneCache.preloadLength = m_preloadLength;
#if UNITY_EDITOR
        ulong sceneGetBegin = Misc.GetTimeNS();
#endif

        SceneData scene = LoadSceneData(time);
        
        // get scene
#if UNITY_EDITOR
        m_dbgSceneGetTime = Misc.NS2MS(Misc.GetTimeNS() - sceneGetBegin);
#endif

        if (scene) {
#if UNITY_EDITOR
            ulong sceneUpdateBegin = Misc.GetTimeNS();
#endif
            // update scene
            this.BeforeUpdateScene();
            this.UpdateScene(scene, updateNonMaterialAssets);
            this.AfterUpdateScene();
#if UNITY_EDITOR
            SetSortEntities(false);

            if (m_config.Profiling) {
                m_dbgSceneUpdateTime = Misc.NS2MS(Misc.GetTimeNS() - sceneUpdateBegin);
                UpdateProfileReport(scene);
            }
#endif
        }
    }

    private SceneData LoadSceneData(float time) {
        SceneData scene = default(SceneData);
        switch (m_playbackMode) {
            case SceneCachePlaybackMode.SnapToPreviousFrame: {
                int frame = Mathf.FloorToInt(time * m_sceneCache.sampleRate);
                scene = m_sceneCache.GetSceneByIndex(frame);
                break;
            }

            case SceneCachePlaybackMode.SnapToNearestFrame: {
                int frame = Mathf.RoundToInt(time * m_sceneCache.sampleRate);
                scene = m_sceneCache.GetSceneByIndex(frame);
                break;
            }
            case SceneCachePlaybackMode.Interpolate: {
                scene = m_sceneCache.GetSceneByTime(m_time, lerp: true);
                break;
            }
        }
        return scene;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    protected override void OnBeforeSerializeMeshSyncPlayerV() {
        
    }

    protected override void OnAfterDeserializeMeshSyncPlayerV() {

        if (m_sceneCachePlayerVersion == CUR_SCENE_CACHE_PLAYER_VERSION)
            return;
        
#if UNITY_EDITOR
        if (m_sceneCachePlayerVersion < (int) SceneCachePlayerVersion.NORMALIZED_PATH_0_9_2) {
            m_sceneCacheFilePath = AssetEditorUtility.NormalizePath(m_sceneCacheFilePath);
        } 
#endif        
        
        m_sceneCachePlayerVersion = CUR_SCENE_CACHE_PLAYER_VERSION;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    

#if UNITY_EDITOR
    void UpdateProfileReport(SceneData data) {
        StringBuilder sb = new System.Text.StringBuilder();
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
        double MBperSec = ((double)prof.sizeEncoded / 1000000.0) / (prof.readTime / 1000.0);
        sb.AppendFormat("  Cache Read: {0:#.##}ms ({1:#.##}MB/sec)\n", prof.readTime, MBperSec);
        sb.AppendFormat("  Cache Decode: {0:#.##}ms (total of worker threads)\n", prof.decodeTime);
        if (prof.setupTime > 0)
            sb.AppendFormat("Setup Scene: {0:#.##}ms\n", prof.setupTime);
        if (prof.lerpTime > 0)
            sb.AppendFormat("Interpolate Scene: {0:#.##}ms\n", prof.lerpTime);
        m_dbgProfileReport = sb.ToString();
    }

    
#endif
    
//----------------------------------------------------------------------------------------------------------------------

    void LogDebug(string logMessage) {
        if (!m_config.Logging)
            return;

        Debug.Log(logMessage); 
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    void ClampTime() {
        m_time = Mathf.Clamp(m_time, m_timeRange.start, m_timeRange.end);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    #region Events
#if UNITY_EDITOR
    void Reset() {
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings(); 
        m_config = new SceneCachePlayerConfig(projectSettings.GetDefaultSceneCachePlayerConfig());            
    }

    void OnValidate() {
        if (!m_sceneCache)
            return;
        
        ClampTime();
    }
#endif

//----------------------------------------------------------------------------------------------------------------------    
    protected override void OnEnable() {
        base.OnEnable();
        
#if UNITY_EDITOR
        m_onMaterialChangedInSceneViewCB += SavePrefabInEditor; 
#endif
        
        m_animator = GetComponent<Animator>();
        if (!string.IsNullOrEmpty(m_sceneCacheFilePath)) {
            OpenCacheInternal(m_sceneCacheFilePath);
        }
        
        if (!m_sceneCache)
            return;
        
        ClampTime();
        
    }

    protected override void OnDisable() {
        base.OnDisable();
#if UNITY_EDITOR
        m_onMaterialChangedInSceneViewCB -= SavePrefabInEditor; 
#endif
        
        CloseCache();
    }

    // note:
    // Update() is called *before* animation update.
    // in many cases m_time is controlled by animation system. so scene update must be handled in LateUpdate()
    void LateUpdate() {
        UpdatePlayer( updateNonMaterialAssets: false);
    }
    #endregion //Events

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private string                 m_sceneCacheFilePath = null; //The full path of the file. Use '/'
    [SerializeField] private SceneCachePlaybackMode m_playbackMode = SceneCachePlaybackMode.SnapToNearestFrame;
    
    [SerializeField] TimeUnit  m_timeUnit      = TimeUnit.Seconds;
    [SerializeField] float     m_time;
    [SerializeField] int       m_frame         = 1;
    [SerializeField] int       m_preloadLength = 1;

    [SerializeField] private SceneCachePlayerConfig m_config;
    
    //only used when the sceneCacheFilePath has a valid importer (under Assets)
    [SerializeField] bool m_overrideModelImporterSettings = false;
    
    //Renamed in 0.10.x-preview
    [FormerlySerializedAs("m_version")] [HideInInspector][SerializeField] private int m_sceneCachePlayerVersion = (int) CUR_SCENE_CACHE_PLAYER_VERSION;
    private const int CUR_SCENE_CACHE_PLAYER_VERSION = (int) SceneCachePlayerVersion.NORMALIZED_PATH_0_9_2;
        
    SceneCacheData m_sceneCache;
    TimeRange      m_timeRange;
    float          m_timePrev = -1;
    Animator       m_animator = null;
    private float m_reqNormalizedTime = 0;

#if UNITY_EDITOR
    [SerializeField] bool m_foldCacheSettings = true;
    float                 m_dbgSceneGetTime;
    float                 m_dbgSceneUpdateTime;
    string                m_dbgProfileReport;
#endif

//----------------------------------------------------------------------------------------------------------------------    
    
    enum SceneCachePlayerVersion {
        NO_VERSIONING     = 0, //Didn't have versioning in earlier versions
        STRING_PATH_0_4_0 = 2, //0.4.0-preview: the path is declared as a string 
        NORMALIZED_PATH_0_9_2 = 3, //0.9.2-preview: Path must be normalized by default 
    
    }
    
}

} //end namespace
