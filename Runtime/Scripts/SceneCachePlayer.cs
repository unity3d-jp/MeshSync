using UnityEngine;

namespace UTJ.MeshSync
{
    [ExecuteInEditMode]
    public class SceneCachePlayer : MonoBehaviour
    {
        #region Fields
        [SerializeField] DataPath m_cacheFilePath = new DataPath();
        [SerializeField] float m_time;
        [SerializeField] bool m_interpolation = true;
        [SerializeField] string m_server = "127.0.0.1";
        [SerializeField] int m_port = 32434;

        SceneCacheData m_sceneCache;
        string m_pathPrev;
        float m_timePrev = -1;
        bool m_needsOpen = false;
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
        public int port
        {
            get { return m_port; }
        }
        #endregion

        #region Public Methods
        bool OpenCache(string path)
        {
            Release();
            m_sceneCache = SceneCacheData.Open(path);
            if (m_sceneCache)
            {
                m_cacheFilePath.fullPath = path;
                return true;
            }
            return false;
        }
        #endregion

        #region Impl
        void Release()
        {
            m_sceneCache.Close();
            m_timePrev = -1;
            m_pathPrev = "";
        }
        #endregion

        #region Events
#if UNITY_EDITOR
        void Reset()
        {
            m_cacheFilePath.isDirectory = false;
        }
        void OnValidate()
        {
            var path = m_cacheFilePath.fullPath;
            if (path != m_pathPrev)
            {
                m_pathPrev = path;
                m_needsOpen = true;
            }
        }
#endif

        void OnEnable()
        {
            m_needsOpen = true;
        }

        void OnDisable()
        {
            Release();
        }

        void Update()
        {
            if (m_needsOpen)
            {
                m_needsOpen = false;
                OpenCache(m_cacheFilePath.fullPath);
            }

            if (m_sceneCache && m_time != m_timePrev)
            {
                m_timePrev = m_time;
                var scene = m_sceneCache.GetSceneByTime(m_time, m_interpolation);
                SceneCacheData.SendScene(m_server, m_port, scene);
            }
        }
        #endregion
    }

}
