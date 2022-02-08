using System;
using System.Collections.Generic;
using System.IO;
using JetBrains.Annotations;
using UnityEngine;
using UnityEngine.Serialization;

namespace Unity.MeshSync.Editor {

[Serializable]
internal class DCCPluginInstallInfo : ISerializationCallbackReceiver {
    
    internal void SetPluginVersion(string appPath, string pluginVersion) {
        m_pluginVersions[appPath] = pluginVersion;
    }

    [CanBeNull]
    internal string GetPluginVersion(string appPath) {
        return m_pluginVersions.TryGetValue(appPath, out string version) ? version : null;
    }
    
    internal void RemovePluginVersion(string appPath) {
        m_pluginVersions.Remove(appPath);
    }
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    public void OnAfterDeserialize() {
        if (null == m_pluginVersions) {            
            m_pluginVersions = new Dictionary<string, string>();
        }

        m_pluginVersions.Clear();
        
        if (null == m_appPathList || m_appPathList.Count <= 0)
            return;

        using (List<string>.Enumerator appPathEnumerator = m_appPathList.GetEnumerator())
        using (List<string>.Enumerator pluginVersionEnumerator = m_pluginVersionList.GetEnumerator()) {
            while (appPathEnumerator.MoveNext() && pluginVersionEnumerator.MoveNext()) {
                string appPath = appPathEnumerator.Current;
                if (string.IsNullOrEmpty(appPath))
                    continue;
            
                m_pluginVersions[appPath] = pluginVersionEnumerator.Current;                
            }
        }
    }
    
    public void OnBeforeSerialize() {
        m_appPathList = new List<string>();
        m_pluginVersionList = new List<string>();

        foreach (KeyValuePair<string, string> kv in m_pluginVersions) {
            m_appPathList.Add(kv.Key);
            m_pluginVersionList.Add(kv.Value);
        }
        
    }

    
//----------------------------------------------------------------------------------------------------------------------    

    internal static string GetInstallInfoPath(DCCToolInfo info) {
        string localAppDataFolder = null;
        
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                localAppDataFolder = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
                break;
            }
            case RuntimePlatform.OSXEditor: {
                string userProfile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                localAppDataFolder = Path.Combine(userProfile, "Library/Application Support");
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                string userProfile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                localAppDataFolder = Path.Combine(userProfile, ".config/unity3d");
                break;
            }
            default: {
                throw new NotImplementedException();
            }
        }

        string desc = info.GetDescription().Replace(' ', '_');
        string installInfoFolder = Path.Combine(localAppDataFolder, "Unity", "MeshSync");
        return Path.Combine(installInfoFolder, $"UnityMeshSyncInstallInfo_{desc}.json");
    }
    
//----------------------------------------------------------------------------------------------------------------------       
   
    [SerializeField] private List<string> m_appPathList;
    [SerializeField] private List<string> m_pluginVersionList;
    
#pragma warning disable 414
    //Renamed in 0.10.x-preview
    [FormerlySerializedAs("m_classVersion")] [SerializeField] private int m_dccPluginInstallInfoVersion = CUR_PLUGIN_INSTALL_INFO_VERSION;
#pragma warning restore 414

    
    private Dictionary<string, string> m_pluginVersions = new Dictionary<string, string>();
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    private const int CUR_PLUGIN_INSTALL_INFO_VERSION  = (int) DCCPluginInstallInfoVersion.APP_LIST_0_5_X;    
    
//----------------------------------------------------------------------------------------------------------------------
    
    enum DCCPluginInstallInfoVersion {
        APP_LIST_0_5_X = 1, //Contains a list of app paths for version 0.5.x-preview
    
    }
}

} //end namespace