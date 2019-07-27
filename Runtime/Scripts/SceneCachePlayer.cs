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
        [SerializeField] bool m_logging = false;

        SceneCacheData m_sceneCache;
        TimeRange m_timeRange;
        string m_pathPrev = "";
        float m_timePrev = -1;
        bool m_openRequested = false;
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
    }

}
