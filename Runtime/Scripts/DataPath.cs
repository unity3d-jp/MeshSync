using System;
using UnityEngine;


namespace Unity.MeshSync  {
[Serializable]
internal class DataPath {
    public enum Root {
        Current,
        PersistentData,
        StreamingAssets,
        TemporaryCache,
        DataPath,
    }

//----------------------------------------------------------------------------------------------------------------------

    internal Root GetRoot() { return m_root; }

    internal string GetLeaf() {  return m_leaf;}

    internal string GetFullPath() {
        UpdateFullPath();
        return m_fullpath;       
    }

    internal void SetFullPath(string fullPath) {
        if (fullPath.Contains(Application.streamingAssetsPath)) {
            m_root = Root.StreamingAssets;
            m_leaf = fullPath.Replace(Application.streamingAssetsPath + "/", "");
        } else if (fullPath.Contains(Application.dataPath)) {
            m_root = Root.DataPath;
            m_leaf = fullPath.Replace(Application.dataPath + "/", "");
        } else if (fullPath.Contains(Application.persistentDataPath)) {
            m_root = Root.PersistentData;
            m_leaf = fullPath.Replace(Application.persistentDataPath + "/", "");
        } else if (fullPath.Contains(Application.temporaryCachePath)) {
            m_root = Root.TemporaryCache;
            m_leaf = fullPath.Replace(Application.temporaryCachePath + "/", "");
        } else {
            m_root = Root.Current;
            m_leaf = fullPath;
        }
        m_dirty = true;        
    }


//----------------------------------------------------------------------------------------------------------------------    

#if UNITY_EDITOR

    internal void SetReadOnly(bool readOnly) { m_readOnly = readOnly; }

    internal void ShowRootSelector(bool show) { m_showRootSelector = show; }

    internal void SetIsDirectory(bool isDirectory) { m_isDirectory = isDirectory;}


#endif

//----------------------------------------------------------------------------------------------------------------------    
    public DataPath() { }
    public DataPath(Root root, string leaf) {
        m_root = root;
        m_leaf = leaf;
    }

    public DataPath(string path) {
        SetFullPath(path);
    }
//----------------------------------------------------------------------------------------------------------------------    

    public bool CreateDirectory() {
        try {
            string path = GetFullPath();
            if (!System.IO.Directory.Exists(path))
                System.IO.Directory.CreateDirectory(path);
            // note: Directory.CreateDirectory() throw exception if the path is a file
        } catch (System.Exception) {
            return false;
        }
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    public override string ToString() {
        return GetFullPath();
    }

//----------------------------------------------------------------------------------------------------------------------    
    void UpdateFullPath() {
        if (!m_dirty)
            return;
        m_dirty = false;

        if (m_root == Root.Current) {
            m_fullpath = m_leaf;
        } else {
            string tmp = "";
            switch (m_root) {
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

//----------------------------------------------------------------------------------------------------------------------        

    [SerializeField] Root   m_root;
    [SerializeField] string m_leaf;
    bool                    m_dirty = true;
    string                  m_fullpath;
#if UNITY_EDITOR
    // just for inspector
    [SerializeField] bool m_readOnly         = false;
    [SerializeField] bool m_showRootSelector = false;
    [SerializeField] bool m_isDirectory      = true;
#endif
    
}

} //end namespace
