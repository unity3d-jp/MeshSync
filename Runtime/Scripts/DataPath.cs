using System;
using UnityEngine;


namespace UTJ.MeshSync
{
    [Serializable]
    public class DataPath
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
#if UNITY_EDITOR
        // just for inspector
        [SerializeField] bool m_readOnly = false;
        [SerializeField] bool m_showRootSelector = false;
#endif

        public Root root
        {
            get { return m_root; }
            set { m_root = value; }
        }
        public string leaf
        {
            get { return m_leaf; }
            set { m_leaf = value; }
        }
        public string fullPath
        {
            get
            {
                if (m_root == Root.Current)
                    return m_leaf;

                string ret = "";
                switch (m_root)
                {
                    case Root.PersistentData:
                        ret = Application.persistentDataPath + "/";
                        break;
                    case Root.StreamingAssets:
                        ret = Application.streamingAssetsPath + "/";
                        break;
                    case Root.TemporaryCache:
                        ret = Application.temporaryCachePath + "/";
                        break;
                    case Root.DataPath:
                        ret = Application.dataPath + "/";
                        break;
                }
                ret += m_leaf;
                return ret;
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
                // exception will be thrown if fullPath is a file
                System.IO.Directory.CreateDirectory(fullPath);
            }
            catch(System.Exception)
            {
                return false;
            }
            return true;
        }

        public override string ToString()
        {
            return fullPath;
        }
    }
}
