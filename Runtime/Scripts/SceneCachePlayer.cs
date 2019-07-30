using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.MeshSync
{
    [RequireComponent(typeof(MeshSyncServer))]
    [ExecuteInEditMode]
    public class SceneCachePlayer : MonoBehaviour
    {
        #region Fields
        [SerializeField] DataPath m_cacheFilePath = new DataPath();
        [SerializeField] float m_time;
        [SerializeField] bool m_interpolation = false;
        [SerializeField] bool m_logging = false;

        SceneCacheData m_sceneCache;
        TimeRange m_timeRange;
        string m_pathPrev = "";
        float m_timePrev = -1;
        bool m_openRequested = false;
        #endregion

        #region Properties
        public DataPath cacheFilePath
        {
            get { return m_cacheFilePath; }
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
        #endregion

        #region Public Methods
        public bool OpenCache(string path)
        {
            CloseCache();
            m_openRequested = false;

            m_sceneCache = SceneCacheData.Open(path);
            if (m_sceneCache)
            {
                m_cacheFilePath.fullPath = path;
                m_pathPrev = path;
                m_timeRange = m_sceneCache.timeRange;
                if (m_logging)
                    Debug.Log(string.Format("SceneCachePlayer: cache opened ({0})", path));
                return true;
            }
            else
            {
                if (m_logging)
                    Debug.Log(string.Format("SceneCachePlayer: cache open failed ({0})", path));
                return false;
            }
        }

        public void CloseCache()
        {
            if (m_sceneCache)
            {
                m_sceneCache.Close();
                if (m_logging)
                    Debug.Log(string.Format("SceneCachePlayer: cache closed ({0})", m_cacheFilePath));
            }
            m_timePrev = -1;
        }

        public bool AddCurve(AnimationClip clip, string path)
        {
            if (!m_sceneCache)
                return false;

            var curve = m_sceneCache.GetTimeCurve(InterpolationMode.Constant);
            if (curve == null)
                return false;
            clip.SetCurve(path, typeof(SceneCachePlayer), "m_time", curve);
            return true;
        }

#if UNITY_EDITOR
        public bool AddAnimator(string assetPath)
        {
            if (m_sceneCache.sceneCount < 2)
                return false;

            var dstPath = string.Format("{0}/{1}.anim", assetPath, gameObject.name);
            var clip = new AnimationClip();
            var sampleRate = m_sceneCache.sampleRate;
            if (sampleRate > 0.0f)
                clip.frameRate = sampleRate;
            AddCurve(clip, "");
            clip = Misc.SaveAsset(clip, dstPath);
            if (clip == null)
                return false;

            var animator = Misc.GetOrAddComponent<Animator>(gameObject);
            animator.runtimeAnimatorController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPathWithClip(dstPath + ".controller", clip);
            return true;
        }
#endif

        public void UpdatePlayer()
        {
            if (m_openRequested)
            {
                OpenCache(m_cacheFilePath.fullPath);
            }

            if (m_sceneCache && m_time != m_timePrev)
            {
                m_timePrev = m_time;
                var scene = m_sceneCache.GetSceneByTime(m_time, m_interpolation);

                var server = GetComponent<MeshSyncServer>();
                server.BeforeUpdateScene();
                server.UpdateScene(scene);
                server.AfterUpdateScene();
            }
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

            m_time = Mathf.Clamp(m_time, m_timeRange.start, m_timeRange.end);
        }
        #endregion

        #region Events
#if UNITY_EDITOR
        void Reset()
        {
            m_cacheFilePath.isDirectory = false;
            m_cacheFilePath.readOnly = true;
            m_cacheFilePath.showRootSelector = true;
            GetComponent<MeshSyncServer>().serverPort = Random.Range(11111, 65000);
        }
        void OnValidate()
        {
            CheckParamsUpdated();
        }
#endif

        void OnEnable()
        {
            CheckParamsUpdated();
        }

        void OnDisable()
        {
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
