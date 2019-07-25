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
        [SerializeField] int m_port = 0;

        SceneCacheData m_sceneCache;
        TimeRange m_timeRange;
        string m_pathPrev;
        float m_timePrev = -1;
        bool m_needsOpen = false;
        #endregion

        #region Properties
        public string cacheFilePath
        {
            get { return m_cacheFilePath.fullPath; }
            set { m_cacheFilePath.fullPath = value; CheckParamsUpdated(); }
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
                m_timeRange = m_sceneCache.timeRange;
                return true;
            }
            return false;
        }
        #endregion


        #region Impl
#if UNITY_EDITOR
        [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
        public static void CreateSceneCachePlayer(MenuCommand menuCommand)
        {
            var path = EditorUtility.OpenFilePanel("Select Cache File", "", "");
            if (path.Length > 0)
            {
                var go = new GameObject();
                go.name = "SceneCachePlayer";

                var server = go.AddComponent<MeshSyncServer>();
                server.rootObject = go.GetComponent<Transform>();
                server.progressiveDisplay = false;

                var player = go.AddComponent<SceneCachePlayer>();
                if (!player.OpenCache(path))
                {
                    Debug.LogError("Failed to open " + path + ".\nPossible reasons: the file is not scene cache, or file format version does not match.");
                    DestroyImmediate(go);
                }
                else
                {
                    Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
                }
            }
        }
#endif

        void Release()
        {
            m_sceneCache.Close();
            m_timePrev = -1;
        }

        void CheckParamsUpdated()
        {
            var path = m_cacheFilePath.fullPath;
            if (path != m_pathPrev)
            {
                m_pathPrev = path;
                m_needsOpen = true;
            }

            m_time = Mathf.Clamp(m_time, m_timeRange.start, m_timeRange.end);
        }
        #endregion

        #region Events
#if UNITY_EDITOR
        void Reset()
        {
            m_cacheFilePath.isDirectory = false;
            m_port = Random.Range(11111, 65000);
            GetComponent<MeshSyncServer>().serverPort = m_port;
        }
        void OnValidate()
        {
            CheckParamsUpdated();
        }
#endif

        void OnEnable()
        {
            m_needsOpen = true;
        }

        void OnDisable()
        {
            Release();
            m_pathPrev = "";
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
                SceneCacheData.SendScene("127.0.0.1", m_port, scene);
                GetComponent<MeshSyncServer>().PollServerEvents();
            }
        }
        #endregion
    }

}
