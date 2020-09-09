using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync
{
    [ExecuteInEditMode]
    internal class SceneCachePlayer : MeshSyncPlayer
    {
        #region Types
        public enum TimeUnit
        {
            Seconds,
            Frames,
        }

        public enum BaseFrame
        {
            Zero = 0,
            One = 1,
        }
        #endregion

        #region Fields
        [SerializeField] DataPath m_cacheFilePath = new DataPath();
        [SerializeField] TimeUnit m_timeUnit = TimeUnit.Seconds;
        [SerializeField] float m_time;
        [SerializeField] bool m_interpolation = false;
        [SerializeField] BaseFrame m_baseFrame = BaseFrame.One;
        [SerializeField] int m_frame = 1;
        [SerializeField] int m_preloadLength = 1;

        SceneCacheData m_sceneCache;
        TimeRange m_timeRange;
        string m_pathPrev = "";
        float m_timePrev = -1;
        bool m_openRequested = false;

#if UNITY_EDITOR
        [SerializeField] bool m_foldCacheSettings = true;
        float m_dbgSceneGetTime;
        float m_dbgSceneUpdateTime;
        string m_dbgProfileReport;
#endif
        #endregion

        #region Properties
        internal DataPath cacheFilePath
        {
            get { return m_cacheFilePath; }
        }
        public int frameCount
        {
            get { return m_sceneCache.sceneCount; }
        }

        public TimeUnit timeUnit
        {
            get { return m_timeUnit; }
            set
            {
                m_timeUnit = value;
                if (m_timeUnit == TimeUnit.Frames)
                    m_interpolation = false;
            }
        }
        public float time
        {
            get { return m_time; }
            set { m_time = value; }
        }
        public bool interpolation
        {
            get { return m_interpolation; }
            set { m_interpolation = value; }
        }
        public BaseFrame baseFrame
        {
            get { return m_baseFrame; }
            set { m_baseFrame = value; }
        }
        public int frame
        {
            get { return m_frame; }
            set { m_frame = value; }
        }
        public int preloadLength
        {
            get { return m_preloadLength; }
            set { m_preloadLength = value; }
        }

#if UNITY_EDITOR
        public bool foldCacheSettings
        {
            get { return m_foldCacheSettings; }
            set { m_foldCacheSettings = value; }
        }
        public string dbgProfileReport
        {
            get { return m_dbgProfileReport; }
        }
#endif
        #endregion

        #region Public Methods
        public bool OpenCache(string path)
        {
            CloseCache();
            m_openRequested = false;

            m_sceneCache = SceneCacheData.Open(path);
            if (m_sceneCache)
            {
#if UNITY_EDITOR
                this.sortEntities = true;
#endif
                m_cacheFilePath.fullPath = path;
                m_pathPrev = path;
                m_timeRange = m_sceneCache.timeRange;
                if (m_config.Logging)
                    Debug.Log(string.Format("SceneCachePlayer: cache opened ({0})", path));
                return true;
            }
            else
            {
                if (m_config.Logging)
                    Debug.Log(string.Format("SceneCachePlayer: cache open failed ({0})", path));
                return false;
            }
        }

        public void CloseCache()
        {
            if (m_sceneCache)
            {
                m_sceneCache.Close();
                if (m_config.Logging)
                    Debug.Log(string.Format("SceneCachePlayer: cache closed ({0})", m_cacheFilePath));
            }
            m_timePrev = -1;
        }

#if UNITY_EDITOR
        public bool ResetTimeAnimation()
        {
            if (m_sceneCache.sceneCount < 2)
                return false;

            var animator = Misc.GetOrAddComponent<Animator>(gameObject);
            AnimationClip clip = null;
            if (animator.runtimeAnimatorController != null)
            {
                var clips = animator.runtimeAnimatorController.animationClips;
                if (clips != null && clips.Length > 0)
                {
                    var tmp = animator.runtimeAnimatorController.animationClips[0];
                    if (tmp != null)
                    {
                        clip = tmp;
                        Undo.RegisterCompleteObjectUndo(clip, "SceneCachePlayer");
                    }
                }
            }

            if (clip == null)
            {
                clip = new AnimationClip();

                var animPath = string.Format("{0}/{1}.anim", assetPath, gameObject.name);
                var controllerPath = string.Format("{0}/{1}.controller", assetPath, gameObject.name);
                clip = Misc.SaveAsset(clip, animPath);
                if (clip == null)
                    return false;

                animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(controllerPath, clip);
            }
            var sampleRate = m_sceneCache.sampleRate;
            if (sampleRate > 0.0f)
                clip.frameRate = sampleRate;

            var tPlayer = typeof(SceneCachePlayer);
            clip.SetCurve("", tPlayer, "m_time", null);
            clip.SetCurve("", tPlayer, "m_frame", null);
            if (m_timeUnit == TimeUnit.Seconds)
            {
                var curve = m_sceneCache.GetTimeCurve(InterpolationMode.Constant);
                clip.SetCurve("", tPlayer, "m_time", curve);
            }
            else if (m_timeUnit == TimeUnit.Frames)
            {
                var curve = m_sceneCache.GetFrameCurve((int)m_baseFrame);
                clip.SetCurve("", tPlayer, "m_frame", curve);
            }

            AssetDatabase.SaveAssets();
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
            return true;
        }
#endif

        public void UpdatePlayer()
        {
            if (m_openRequested)
            {
                OpenCache(m_cacheFilePath.fullPath);
            }

            if (m_timeUnit == TimeUnit.Frames)
            {
                int offset = (int)m_baseFrame;
                m_frame = Mathf.Clamp(m_frame, offset, frameCount + offset);
                m_time = m_sceneCache.GetTime(m_frame - offset);
            }

            if (m_sceneCache)
            {
                if (m_time != m_timePrev)
                {
                    m_timePrev = m_time;
                    m_sceneCache.preloadLength = m_preloadLength;
#if UNITY_EDITOR
                    ulong sceneGetBegin = Misc.GetTimeNS();
#endif
                    // get scene
                    var scene = m_sceneCache.GetSceneByTime(m_time, m_interpolation);
#if UNITY_EDITOR
                    m_dbgSceneGetTime = Misc.NS2MS(Misc.GetTimeNS() - sceneGetBegin);
#endif

                    if (scene)
                    {
#if UNITY_EDITOR
                        ulong sceneUpdateBegin = Misc.GetTimeNS();
#endif
                        // update scene
                        this.BeforeUpdateScene();
                        this.UpdateScene(scene);
                        this.AfterUpdateScene();
#if UNITY_EDITOR
                        this.sortEntities = false;

                        if (m_config.Profiling)
                        {
                            m_dbgSceneUpdateTime = Misc.NS2MS(Misc.GetTimeNS() - sceneUpdateBegin);
                            UpdateProfileReport(scene);
                        }
#endif
                    }
                }
                else if(m_sceneCache.preloadLength != m_preloadLength)
                {
                    m_sceneCache.preloadLength = m_preloadLength;
                    m_sceneCache.Preload(m_sceneCache.GetFrame(m_time));
                }
            }
        }

        public SceneData GetLastScene()
        {
            if (m_sceneCache)
                return m_sceneCache.GetSceneByTime(m_timePrev, m_interpolation);
            return default(SceneData);
        }
        #endregion

        #region Impl
        void CheckParamsUpdated()
        {
            var path = m_cacheFilePath.fullPath;
            if (path != m_pathPrev)
            {
                m_pathPrev = path;
                m_openRequested = true;
            }

            if (m_sceneCache)
                m_time = Mathf.Clamp(m_time, m_timeRange.start, m_timeRange.end);
        }

#if UNITY_EDITOR
        void UpdateProfileReport(SceneData data)
        {
            var sb = new System.Text.StringBuilder();
            var prof = data.profileData;
            sb.AppendFormat("Scene Get: {0:#.##}ms\n", m_dbgSceneGetTime);
            sb.AppendFormat("Scene Update: {0:#.##}ms\n", m_dbgSceneUpdateTime);
            sb.AppendFormat("\n");

            {
                var sizeEncoded = prof.sizeEncoded;
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
        #endregion

        #region Events
#if UNITY_EDITOR
        void Reset()
        {
            m_cacheFilePath.isDirectory = false;
            m_cacheFilePath.readOnly = true;
            m_cacheFilePath.showRootSelector = true;

            m_config = MeshSyncRuntimeSettings.CreatePlayerConfig(MeshSyncPlayerType.CACHE_PLAYER);            
        }

        void OnValidate()
        {
            CheckParamsUpdated();
        }
#endif

        protected override void OnEnable()
        {
            base.OnEnable();
            CheckParamsUpdated();
        }

        protected override void OnDisable()
        {
            base.OnDisable();
            CloseCache();
            m_pathPrev = "";
        }

        // note:
        // Update() is called *before* animation update.
        // in many cases m_time is controlled by animation system. so scene update must be handled in LateUpdate()
        void LateUpdate()
        {
            UpdatePlayer();
        }
        #endregion
    }

}
