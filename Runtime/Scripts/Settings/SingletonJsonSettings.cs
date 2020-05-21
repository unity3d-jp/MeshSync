using System;
using System.IO;
using Unity.AnimeToolbox;
using UnityEngine;


namespace Unity.MeshSync {

[Serializable]
internal class SingletonJsonSettings<T> where T: class , new() {

    internal static T GetOrCreateSettings() {
#if UNITY_EDITOR
        T settings = LoadSettings();
        if (null != settings) {
            return settings;
        }
#endif
       
        settings = new T();

#if UNITY_EDITOR
        SaveSettings(settings);
#endif
        return settings;            
    }
    
//----------------------------------------------------------------------------------------------------------------------

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