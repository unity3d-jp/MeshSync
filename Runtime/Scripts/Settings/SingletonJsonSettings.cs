using System;
using System.IO;
using Unity.AnimeToolbox;
using UnityEngine;


namespace Unity.MeshSync {

[Serializable]
internal class SingletonJsonSettings<T> where T: class , new() {
    
    internal static T GetOrCreateSettings() {

        if (null != m_settings) {
            return m_settings;
        }

        lock (m_settingsLock) {           
        
#if UNITY_EDITOR
            m_settings = LoadSettings();
            if (null != m_settings) {
                return m_settings;
            }
            m_settings = new T();
            SaveSettings(m_settings);
#else
            m_settings = new T();
#endif
        }        
        return m_settings;            
    }
    
//----------------------------------------------------------------------------------------------------------------------
#if UNITY_EDITOR

    #region File Load/Save for Serialization/deserialization
    static T LoadSettings() {
        string path = GetSettingsPath();
        if (string.IsNullOrEmpty(path) || !File.Exists(path)) {
            return null;
        }
        return FileUtility.DeserializeFromJson<T>(path);
    }
    
    static bool SaveSettings(T obj) {
        string path = GetSettingsPath();
        if (string.IsNullOrEmpty(path)) {
            Debug.LogError("[MeshSync] Missing PathAttribute for " + typeof(T).ToString());
            return false;
        }

        string dir = Path.GetDirectoryName(path);
        if (!string.IsNullOrEmpty(dir)) {
            Directory.CreateDirectory(dir);
           
        }
        
        FileUtility.SerializeToJson<T>(obj, path);
        return true;
    }
    #endregion

#endif 
    
//----------------------------------------------------------------------------------------------------------------------
    private static string GetSettingsPath()  {
        foreach(var attr in typeof(T).GetCustomAttributes(true)) {
            PathAttribute pathAttribute = attr as PathAttribute;
            if (null == pathAttribute)
                continue;

            return pathAttribute.GetPath();
        }
        return null;
    }

    
    
//----------------------------------------------------------------------------------------------------------------------

    private static T m_settings = null;
    private static readonly object m_settingsLock = new object();
}

[AttributeUsage(AttributeTargets.Class)]
internal class PathAttribute : Attribute {
      
    public PathAttribute(string path) {
        if (string.IsNullOrEmpty(path)) {
            Debug.LogError("[MeshSync] path is null or empty.");
            return;
        }

        m_path = path;
    }

    internal string GetPath() { return m_path; }

    private readonly string m_path;
}


} //end namespace