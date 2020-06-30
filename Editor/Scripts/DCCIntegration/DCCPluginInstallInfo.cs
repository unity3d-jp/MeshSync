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

    /// <summary>
    /// MeshSync plugin version installed for this DCC tool
    /// </summary>
    public string         PluginVersion;

    [SerializeField] internal readonly int ClassVersion = 1;

}

} //end namespace