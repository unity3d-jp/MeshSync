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
        
        string dccDesc = m_dccToolInfo.GetDescription();

        string progressBarInfo = "Installing plugin for " + dccDesc;
        EditorUtility.DisplayProgressBar("MeshSync", progressBarInfo,0);
        downloader.Execute((string pluginVersion, List<string> dccPluginLocalPaths) => {

            EditorUtility.DisplayProgressBar("MeshSync", progressBarInfo, 0.5f);
            string configFolder = FindConfigFolder();
            DCCPluginInstallInfo installInfo = null;
            if (dccPluginLocalPaths.Count >0 && File.Exists(dccPluginLocalPaths[0])) {
                installInfo = ConfigureDCCTool(m_dccToolInfo, configFolder, dccPluginLocalPaths[0]);
            }

            if (null == installInfo) {
                Debug.LogError("[MeshSync] Unknown error when installing plugin for " + dccDesc);
            } else {

                //Write DCCPluginInstallInfo for the version
                string installInfoPath = GetInstallInfoPath(configFolder, m_dccToolInfo.DCCToolVersion);
                FileUtility.SerializeToJson(installInfo, installInfoPath);
            }
            EditorUtility.ClearProgressBar();

            onComplete();
        }, () => {
            Debug.LogError("[MeshSync] Failed to download DCC Plugin for " + dccDesc);
            EditorUtility.ClearProgressBar();
        });
    }

//----------------------------------------------------------------------------------------------------------------------    
    internal DCCPluginInstallInfo FindInstallInfo() {
        
        string path = GetInstallInfoPath(FindConfigFolder(), m_dccToolInfo.DCCToolVersion);
        if (!File.Exists(path))
            return null;

        return FileUtility.DeserializeFromJson<DCCPluginInstallInfo>(path);
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    internal DCCToolInfo GetDCCToolInfo() { return m_dccToolInfo; }

//----------------------------------------------------------------------------------------------------------------------    

    //The name of the DCCTool in the filename of the DCC plugin
    protected abstract string GetDCCToolName();

    //returns null when failed
    protected abstract DCCPluginInstallInfo ConfigureDCCTool( DCCToolInfo dccToolInfo, 
        string dccConfigFolder, string localPluginPath);

    protected abstract string FindConfigFolder();
    
//----------------------------------------------------------------------------------------------------------------------    

    private string GetDCCPluginFileName() {
        return GetDCCToolName() + "_" + GetCurrentDCCPluginPlatform() + ".zip";
        
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    private static string GetInstallInfoPath(string dccConfigFolder, string dccToolVersion) {
        const string INSTALL_INFO_FILENAME = "UnityMeshSyncInstallInfo";
        return Path.Combine(dccConfigFolder, INSTALL_INFO_FILENAME + dccToolVersion + ".json");
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
    
}

} //end namespace