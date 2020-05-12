using System;
using System.IO;
using UnityEngine;

namespace UnityEditor.MeshSync {

[Serializable]
internal class DCCPluginInstallInfo {

    internal DCCPluginInstallInfo(string pluginVersion) {
        PluginVersion = pluginVersion;
    }

//----------------------------------------------------------------------------------------------------------------------    

    internal static string GetInstallInfoPath(DCCToolInfo info) {
        string localAppDataFolder = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);

        switch (Application.platform) {
            case RuntimePlatform.OSXEditor: {
                string userProfile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                localAppDataFolder = Path.Combine(userProfile, "Library/Application Support");
                break;
            }
            case RuntimePlatform.LinuxEditor: 
                throw new NotImplementedException();
            default: {
                break;
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

    [SerializeField] internal int ClassVersion = 1;

}

} //end namespace