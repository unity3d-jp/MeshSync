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

        string dccToolName = GetDCCToolInFileNameV();
        string dccPluginFileName = dccToolName + "_" + GetCurrentDCCPluginPlatform() + ".zip";
    
        //Make sure the DCC plugin zip file exists first
        DCCPluginDownloader downloader = new DCCPluginDownloader(false,SAVED_PLUGINS_FOLDER, 
            new string[] { dccPluginFileName }
        );
        
        string dccDesc = m_dccToolInfo.GetDescription();

        string progressBarInfo = "Installing plugin for " + dccDesc;
        EditorUtility.DisplayProgressBar("MeshSync", progressBarInfo,0);
        downloader.Execute((string pluginVersion, List<string> dccPluginLocalPaths) => {

            EditorUtility.DisplayProgressBar("MeshSync", progressBarInfo, 0.5f);
            bool dccConfigured = false;
            if (dccPluginLocalPaths.Count >0 && File.Exists(dccPluginLocalPaths[0])) {
                dccConfigured = ConfigureDCCToolV(m_dccToolInfo, dccPluginLocalPaths[0]);
            }
            
            if (!dccConfigured) {
                HandleFailedIntegration("Failed in configuring DCC ", dccDesc);
                return;
            }
            
            DCCPluginInstallInfo installInfo = new DCCPluginInstallInfo(pluginVersion);

            string installInfoPath = DCCPluginInstallInfo.GetInstallInfoPath(m_dccToolInfo);
            string installInfoFolder = Path.GetDirectoryName(installInfoPath);
            if (null == installInfoPath || null == installInfoFolder) {
                HandleFailedIntegration($"Invalid path: {installInfoPath}",dccDesc);
                return;
            }

            //Write DCCPluginInstallInfo for the version
            Directory.CreateDirectory(installInfoFolder);                

            try {
                FileUtility.SerializeToJson(installInfo, installInfoPath);
            } catch (Exception e) {
                HandleFailedIntegration(e.ToString(), dccDesc);
                return;
            }
            
            EditorUtility.ClearProgressBar();
            FinalizeDCCConfigurationV();
            
            onComplete();
        }, () => {
            Debug.LogError("[MeshSync] Failed to download DCC Plugin for " + dccDesc);
            EditorUtility.ClearProgressBar();
        });
    }

//----------------------------------------------------------------------------------------------------------------------    
    internal DCCPluginInstallInfo FindInstallInfo() {
        
        string path = DCCPluginInstallInfo.GetInstallInfoPath(m_dccToolInfo);
        if (!File.Exists(path))
            return null;

        return FileUtility.DeserializeFromJson<DCCPluginInstallInfo>(path);
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    internal DCCToolInfo GetDCCToolInfo() { return m_dccToolInfo; }

//----------------------------------------------------------------------------------------------------------------------    

    //The name of the DCCTool in the filename of the DCC plugin
    protected abstract string GetDCCToolInFileNameV();

    //returns null when failed
    protected abstract bool ConfigureDCCToolV( DCCToolInfo dccToolInfo, string localPluginPath);
    
    protected abstract void FinalizeDCCConfigurationV();
    
    
//----------------------------------------------------------------------------------------------------------------------    
    private static string GetCurrentDCCPluginPlatform() {
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

    private static void HandleFailedIntegration(string errMessage, string dccDesc) {
        EditorUtility.ClearProgressBar();
        Debug.LogError($"[MeshSync] Error: {errMessage}, when installing plugin for " + dccDesc);
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    

    private readonly DCCToolInfo m_dccToolInfo = null;

    private static readonly string SAVED_PLUGINS_FOLDER = Path.Combine(Application.dataPath, "MeshSyncDCCPlugins~");
    
}

} //end namespace