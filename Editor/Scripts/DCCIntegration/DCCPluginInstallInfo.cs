using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.Analytics;

namespace Unity.MeshSync.Editor {

[Serializable]
internal class DCCPluginInstallInfo : ISerializationCallbackReceiver {
    
    internal void SetPluginVersion(string appPath, string pluginVersion) {
        m_pluginVersions[appPath] = pluginVersion;
    }

    internal string GetPluginVersion(string appPath) {
        if (m_pluginVersions.ContainsKey(appPath))
            return m_pluginVersions[appPath];

        //[TODO-sin: 2020-10-28] Should remove PluginVersion in 1.0.0-preview
        return PluginVersion;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    public void OnAfterDeserialize() {
        m_pluginVersions = new Dictionary<string, string>();
        
        if (null == m_appPathList || m_appPathList.Count <= 0)
            return;

        using (List<string>.Enumerator appPathEnumerator = m_appPathList.GetEnumerator())
        using (List<string>.Enumerator pluginVersionEnumerator = m_appPathList.GetEnumerator()) {
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

    //Obsolete
    [SerializeField] private string PluginVersion;
   
    [SerializeField] private List<string> m_appPathList;
    [SerializeField] private List<string> m_pluginVersionList;
    
#pragma warning disable 414
    [SerializeField] private int m_classVersion = CUR_PLUGIN_INSTALL_INFO_VERSION;
#pragma warning restore 414

    
    private Dictionary<string, string> m_pluginVersions = null;
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    private const            int CUR_PLUGIN_INSTALL_INFO_VERSION  = (int) DCCPluginInstallInfoVersion.APP_LIST_0_5_X;    
    
//----------------------------------------------------------------------------------------------------------------------
    
    enum DCCPluginInstallInfoVersion {
        APP_LIST_0_5_X = 1, //Contains a list of app paths for version 0.4.x-preview
    
    }
}

} //end namespace