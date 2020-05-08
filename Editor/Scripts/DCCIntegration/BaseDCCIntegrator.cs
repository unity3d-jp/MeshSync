using System;
using System.Collections.Generic;
using System.IO;
using Unity.AnimeToolbox;
using UnityEngine;

namespace UnityEditor.MeshSync {

internal abstract class BaseDCCIntegrator {

    internal BaseDCCIntegrator(DCCToolInfo dccToolInfo) {
        m_dccToolInfo = dccToolInfo;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    internal void Integrate(Action onComplete) {
        string dccPluginFileName = GetDCCPluginFileName();
    
        //Make sure the DCC plugin zip file exists first
        DCCPluginDownloader downloader = new DCCPluginDownloader(false,SAVED_PLUGINS_FOLDER, 
            new string[] { dccPluginFileName }
        );

        string dccName = GetDCCToolName();
        EditorUtility.DisplayProgressBar("MeshSync", "Installing plugin for " + dccName,0);
        downloader.Execute((string pluginVersion, List<string> dccPluginLocalPaths) =>
        {

            string configFolder = null;
            if (dccPluginLocalPaths.Count >0 && File.Exists(dccPluginLocalPaths[0])) {
                configFolder = ConfigureDCCTool(dccPluginLocalPaths[0]);
            }

            if (string.IsNullOrEmpty(configFolder)) {
                Debug.LogError("[MeshSync] Unknown error when installing plugin for " + dccName);
            } else {
                //Write DCCPluginInstallInfo for the version
                DCCPluginInstallInfo installInfo = new DCCPluginInstallInfo(pluginVersion);
                FileUtility.SerializeToJson(installInfo, Path.Combine(configFolder,INSTALL_INFO_FILENAME));
            }
            EditorUtility.ClearProgressBar();

            onComplete();
        }, () => {
            Debug.LogError("Failed to download DCC Plugin for " + dccName);
            EditorUtility.ClearProgressBar();
        });
    }

//----------------------------------------------------------------------------------------------------------------------    
    internal DCCPluginInstallInfo FindInstallInfo() {
        string path = Path.Combine(FindConfigFolder(), INSTALL_INFO_FILENAME);
        if (!File.Exists(path))
            return null;

        return FileUtility.DeserializeFromJson<DCCPluginInstallInfo>(path);
    }
    
//----------------------------------------------------------------------------------------------------------------------    

    protected abstract string GetDCCToolName();

    //returns null when failed
    protected abstract string ConfigureDCCTool(string localPluginPath);

    protected abstract string FindConfigFolder();
    
//----------------------------------------------------------------------------------------------------------------------    

    private string GetDCCPluginFileName() {
        return GetDCCToolName() + "_" + GetCurrentDCCPluginPlatform() + ".zip";
        
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    static string GetCurrentDCCPluginPlatform() {
        string platform = null;
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: platform = "Windows"; break;
            case RuntimePlatform.OSXEditor:  platform = "Mac"; break;
            case RuntimePlatform.LinuxEditor:  platform = "Linux"; break;
            default: {
                throw new NotImplementedException();
            }
        }

        return platform;

    }
    
//----------------------------------------------------------------------------------------------------------------------    

    private readonly DCCToolInfo m_dccToolInfo = null;

    private static readonly string SAVED_PLUGINS_FOLDER = Path.Combine(Application.dataPath, "MeshSyncDCCPlugins~");
    private const string INSTALL_INFO_FILENAME = "UnityMeshSyncInstallInfo.json";
    
}

} //end namespace