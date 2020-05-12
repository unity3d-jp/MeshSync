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

    internal static string GetInstallInfoPath(string dccToolName, string dccToolVersion) {
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

        string installInfoFolder = Path.Combine(localAppDataFolder, "Unity", "MeshSync");
        return Path.Combine(installInfoFolder, $"UnityMeshSyncInstallInfo_{dccToolName}_{dccToolVersion}.json");
    }    
    
    
//----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// MeshSync plugin version installed for this DCC tool
    /// </summary>
    public string         PluginVersion;

    [SerializeField] internal int ClassVersion = 1;

}

} //end namespace