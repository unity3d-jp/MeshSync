using System;
using System.IO;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[Serializable]
internal class DCCPluginInstallInfo {

    internal DCCPluginInstallInfo(string pluginVersion) {
        PluginVersion = pluginVersion;
    }

//----------------------------------------------------------------------------------------------------------------------    
    internal string GetPluginVersion(string appPath) {
        return PluginVersion;
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
   
#pragma warning disable 414
    [SerializeField] private int m_version = CUR_PLUGIN_INSTALL_INFO_VERSION;
#pragma warning restore 414
    
    private const            int CUR_PLUGIN_INSTALL_INFO_VERSION  = (int) DCCPluginInstallInfoVersion.APP_LIST_0_4_X;
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    enum DCCPluginInstallInfoVersion {
        APP_LIST_0_4_X = 1, //Contains a list of app paths for version 0.4.x-preview
    
    }
}

} //end namespace