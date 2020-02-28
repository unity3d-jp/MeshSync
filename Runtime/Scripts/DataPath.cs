using System;
using UnityEngine;


namespace Unity.MeshSync
{
    [Serializable]
    internal class DataPath
    {
        public enum Root
        {
            Current,
            PersistentData,
            StreamingAssets,
            TemporaryCache,
            DataPath,
        }

        [SerializeField] Root m_root;
        [SerializeField] string m_leaf;
        bool m_dirty = true;
        string m_fullpath;
#if UNITY_EDITOR
        // just for inspector
        [SerializeField] bool m_readOnly = false;
        [SerializeField] bool m_showRootSelector = false;
        [SerializeField] bool m_isDirectory = true;
#endif

        public Root root
        {
            get { return m_root; }
            set { m_root = value; m_dirty = true; }
        }
        public string leaf
        {
            get { return m_leaf; }
            set { m_leaf = value; m_dirty = true; }
        }
        public string fullPath
        {
            get
            {
                UpdateFullPath();
                return m_fullpath;
            }

            set
            {
                if (value.Contains(Application.streamingAssetsPath))
                {
                    m_root = Root.StreamingAssets;
                    m_leaf = value.Replace(Application.streamingAssetsPath + "/", "");
                }
                else if (value.Contains(Application.dataPath))
                {
                    m_root = Root.DataPath;
                    m_leaf = value.Replace(Application.dataPath + "/", "");
                }
                else if (value.Contains(Application.persistentDataPath))
                {
                    m_root = Root.PersistentData;
                    m_leaf = value.Replace(Application.persistentDataPath + "/", "");
                }
                else if (value.Contains(Application.temporaryCachePath))
                {
                    m_root = Root.TemporaryCache;
                    m_leaf = value.Replace(Application.temporaryCachePath + "/", "");
                }
                else
                {
                    m_root = Root.Current;
                    m_leaf = value;
                }
                m_dirty = true;
            }
        }
        public bool exists
        {
            get
            {
                return System.IO.Directory.Exists(fullPath);
            }
        }


#if UNITY_EDITOR
        public bool readOnly
        {
            get { return m_readOnly; }
            set { m_readOnly = value; }
        }
        public bool showRootSelector
        {
            get { return m_showRootSelector; }
            set { m_showRootSelector = value; }
        }
        public bool isDirectory
        {
            get { return m_isDirectory; }
            set { m_isDirectory = value; }
        }
#endif

        public DataPath() { }
        public DataPath(Root root, string leaf)
        {
            m_root = root;
            m_leaf = leaf;
        }

        public DataPath(string path)
        {
            fullPath = path;
        }

        public bool CreateDirectory()
        {
            try
            {
                var path = fullPath;
                if (!System.IO.Directory.Exists(path))
                    System.IO.Directory.CreateDirectory(path);
                // note: Directory.CreateDirectory() throw exception if the path is a file
            }
            catch (System.Exception)
            {
                return false;
            }
            return true;
        }

        public override string ToString()
        {
            return fullPath;
        }

        void UpdateFullPath()
        {
            if (!m_dirty)
                return;
            m_dirty = false;

            if (m_root == Root.Current)
            {
                m_fullpath = m_leaf;
            }
            else
            {
                string tmp = "";
                switch (m_root)
                {
                    case Root.PersistentData:
                        tmp = Application.persistentDataPath + "/";
                        break;
                    case Root.StreamingAssets:
                        tmp = Application.streamingAssetsPath + "/";
                        break;
                    case Root.TemporaryCache:
                        tmp = Application.temporaryCachePath + "/";
                        break;
                    case Root.DataPath:
                        tmp = Application.dataPath + "/";
                        break;
                }
                tmp += m_leaf;
                m_fullpath = tmp;
            }
        }
    }
}
