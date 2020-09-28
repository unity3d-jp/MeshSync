using System;
using System.Text;
using UnityEngine;
using UnityEngine.Assertions;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync
{

[ExecuteInEditMode]
internal class SceneCachePlayer : MeshSyncPlayer {
    #region Types
    public enum TimeUnit {
        Seconds,
        Frames,
    }

    public enum BaseFrame {
        Zero = 0,
        One = 1,
    }
    
    //[TODO-sin: 2020-9-25] Move to another package?
    private enum LogType {
        DEBUG,
        WARNING,
        ERROR,
    }
    
    #endregion


//----------------------------------------------------------------------------------------------------------------------
    
    SceneCachePlayer() : base() {
        SetSaveAssetsInScene(false);
        MarkMeshesDynamic(true);
    }

    protected override void InitInternalV() {
        
    }
    
    
//----------------------------------------------------------------------------------------------------------------------

    internal string GetSceneCacheFilePath() { return m_sceneCacheFilePath; }
    internal bool IsSceneCacheOpened() { return m_sceneCache;}
    
//----------------------------------------------------------------------------------------------------------------------
    #region Properties
    public int frameCount {
        get { return m_sceneCache.sceneCount; }
    }

    public TimeUnit timeUnit {
        get { return m_timeUnit; }
        set
        {
            m_timeUnit = value;
            if (m_timeUnit == TimeUnit.Frames)
                m_interpolation = false;
        }
    }
    public float time {
        get { return m_time; }
        set { m_time = value; }
    }
    public bool interpolation {
        get { return m_interpolation; }
        set { m_interpolation = value; }
    }
    public BaseFrame baseFrame {
        get { return m_baseFrame; }
        set { m_baseFrame = value; }
    }
    public int frame {
        get { return m_frame; }
        set { m_frame = value; }
    }
    public int preloadLength {
        get { return m_preloadLength; }
        set { m_preloadLength = value; }
    }

#if UNITY_EDITOR
    public bool foldCacheSettings {
        get { return m_foldCacheSettings; }
        set { m_foldCacheSettings = value; }
    }
    public string dbgProfileReport {
        get { return m_dbgProfileReport; }
    }
#endif
    #endregion

    #region Internal Methods

#if UNITY_EDITOR    
    internal bool OpenCacheInEditor(string path) {

        if (!OpenCacheInternal(path)) {
            return false;
        }

        //Delete all children
        if (gameObject.IsPrefabInstance()) {
            PrefabUtility.UnpackPrefabInstance(gameObject, PrefabUnpackMode.Completely, InteractionMode.AutomatedAction);            
        } 
        gameObject.DestroyChildrenImmediate();

        //Initialization after opening a cache file
        m_sceneCacheFilePath = path;
        m_timeRange = m_sceneCache.timeRange;
        
        UpdatePlayer(/* updateNonMaterialAssets = */ true);
        ExportMaterials(false, true);
        ResetTimeAnimation();
        
        SceneData scene = GetLastScene();
        if (!scene.submeshesHaveUniqueMaterial) {
            m_config.SyncMaterialList = false;
        }
        
        return true;
    }
#endif //UNITY_EDITOR    

    private bool ReopenCache() {
        Assert.IsFalse(string.IsNullOrEmpty(m_sceneCacheFilePath));
        return OpenCacheInternal(m_sceneCacheFilePath);
    }

    private bool OpenCacheInternal(string path) {
        CloseCache();

        m_sceneCache = SceneCacheData.Open(path);
        if (!m_sceneCache) {
            Log($"SceneCachePlayer: cache open failed ({path})", LogType.ERROR);
            return false;            
        }
        
#if UNITY_EDITOR
        SetSortEntities(true);
#endif
        Log($"SceneCachePlayer: cache opened ({path})", LogType.DEBUG);
        return true;
    }
    
    internal void CloseCache() {
        if (m_sceneCache) {
            m_sceneCache.Close();
            Log($"SceneCachePlayer: cache closed ({m_sceneCacheFilePath})");
        }
        m_timePrev = -1;
    }

#if UNITY_EDITOR
    internal bool ResetTimeAnimation() {
        if (m_sceneCache.sceneCount < 2)
            return false;

        Animator animator = Misc.GetOrAddComponent<Animator>(gameObject);
        AnimationClip clip = null;
        if (animator.runtimeAnimatorController != null) {
            AnimationClip[] clips = animator.runtimeAnimatorController.animationClips;
            if (clips != null && clips.Length > 0) {
                AnimationClip tmp = animator.runtimeAnimatorController.animationClips[0];
                if (tmp != null) {
                    clip = tmp;
                    Undo.RegisterCompleteObjectUndo(clip, "SceneCachePlayer");
                }
            }
        }

        if (clip == null) {
            clip = new AnimationClip();

            string assetsFolder = GetAssetsFolder();

            string animPath       = string.Format("{0}/{1}.anim", assetsFolder, gameObject.name);
            string controllerPath = string.Format("{0}/{1}.controller", assetsFolder, gameObject.name);
            clip = Misc.SaveAsset(clip, animPath);
            if (clip == null)
                return false;

            animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(controllerPath, clip);
        }
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
            AnimationCurve curve = m_sceneCache.GetFrameCurve((int)m_baseFrame);
            clip.SetCurve("", tPlayer, "m_frame", curve);
        }

        AssetDatabase.SaveAssets();
        UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        return true;
    }
#endif

    private void UpdatePlayer(bool updateNonMaterialAssets) {

        if (m_timeUnit == TimeUnit.Frames) {
            int offset = (int)m_baseFrame;
            m_frame = Mathf.Clamp(m_frame, offset, frameCount + offset);
            m_time = m_sceneCache.GetTime(m_frame - offset);
        }

        if (m_sceneCache) {
            if (m_time != m_timePrev) {
                m_timePrev = m_time;
                m_sceneCache.preloadLength = m_preloadLength;
#if UNITY_EDITOR
                ulong sceneGetBegin = Misc.GetTimeNS();
#endif
                // get scene
                SceneData scene = m_sceneCache.GetSceneByTime(m_time, m_interpolation);
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
            } else if(m_sceneCache.preloadLength != m_preloadLength) {
                m_sceneCache.preloadLength = m_preloadLength;
                m_sceneCache.Preload(m_sceneCache.GetFrame(m_time));
            }
        }

    }

    public SceneData GetLastScene() {
        if (m_sceneCache)
            return m_sceneCache.GetSceneByTime(m_timePrev, m_interpolation);
        return default(SceneData);
    }
    #endregion
   
//----------------------------------------------------------------------------------------------------------------------

    protected override void OnBeforeSerializeMeshSyncPlayerV() {
        
    }

    protected override void OnAfterDeserializeMeshSyncPlayerV() {
        
        
        if (m_version < (int) SceneCachePlayerVersion.STRING_PATH_0_4_0) {
            Assert.IsNotNull(m_cacheFilePath);           
            m_sceneCacheFilePath = m_cacheFilePath.GetFullPath();
        } 
        
        m_version = CUR_SCENE_CACHE_PLAYER_VERSION;
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

    void Log(string logMessage, LogType logType = LogType.DEBUG) {
        if (!m_config.Logging)
            return;

        switch (logType) {
            case LogType.DEBUG: Debug.Log(logMessage); break; 
            case LogType.WARNING: Debug.LogWarning(logMessage); break; 
            case LogType.ERROR: Debug.LogError(logMessage); break;
            default: break;
                
        }        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    void ClampTime() {
        if (m_sceneCache) {
            m_time = Mathf.Clamp(m_time, m_timeRange.start, m_timeRange.end);
        }
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    #region Events
#if UNITY_EDITOR
    void Reset() {
        m_config = MeshSyncRuntimeSettings.CreatePlayerConfig(MeshSyncPlayerType.CACHE_PLAYER);            
    }

    void OnValidate() {
        ClampTime();
    }
#endif

//----------------------------------------------------------------------------------------------------------------------    
    protected override void OnEnable() {
        base.OnEnable();
        if (!string.IsNullOrEmpty(m_sceneCacheFilePath)) {
            ReopenCache();
        }
        
        ClampTime();
    }

    protected override void OnDisable() {
        base.OnDisable();
        CloseCache();
    }

    // note:
    // Update() is called *before* animation update.
    // in many cases m_time is controlled by animation system. so scene update must be handled in LateUpdate()
    void LateUpdate() {
        UpdatePlayer( /*updateNonMaterialAssets = */ false);
    }
    #endregion

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] string    m_sceneCacheFilePath = null;
    [SerializeField] DataPath  m_cacheFilePath = null; //OBSOLETE
    [SerializeField] TimeUnit  m_timeUnit      = TimeUnit.Seconds;
    [SerializeField] float     m_time;
    [SerializeField] bool      m_interpolation = false;
    [SerializeField] BaseFrame m_baseFrame     = BaseFrame.One;
    [SerializeField] int       m_frame         = 1;
    [SerializeField] int       m_preloadLength = 1;

    
    [HideInInspector][SerializeField] private int m_version = (int) CUR_SCENE_CACHE_PLAYER_VERSION;
    private const int CUR_SCENE_CACHE_PLAYER_VERSION = (int) SceneCachePlayerVersion.STRING_PATH_0_4_0;
    
    
    SceneCacheData m_sceneCache;
    TimeRange      m_timeRange;
    float          m_timePrev      = -1;

#if UNITY_EDITOR
    [SerializeField] bool m_foldCacheSettings = true;
    float                 m_dbgSceneGetTime;
    float                 m_dbgSceneUpdateTime;
    string                m_dbgProfileReport;
#endif

//----------------------------------------------------------------------------------------------------------------------    
    
    enum SceneCachePlayerVersion {
        NO_VERSIONING = 0, //Didn't have versioning in earlier versions
        STRING_PATH_0_4_0 = 2, //0.4.0-preview: the path is declared as a string 
    
    }
    
}

} //end namespace
