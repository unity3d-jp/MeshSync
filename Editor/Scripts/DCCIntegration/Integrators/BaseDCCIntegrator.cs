using System;
using System.Collections.Generic;
using System.IO;
using Unity.FilmInternalUtilities;
using Unity.SharpZipLib.Utils;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

internal abstract class BaseDCCIntegrator {

    internal BaseDCCIntegrator(DCCToolInfo dccToolInfo) {
        m_dccToolInfo = dccToolInfo;
    }

//----------------------------------------------------------------------------------------------------------------------    
    internal void Integrate(string requestedPluginVersion, Action onComplete) {

        string dccToolName = GetDCCToolInFileNameV();
        string dccPluginFileName = dccToolName + "_" + GetCurrentDCCPluginPlatform() + ".zip";
    
        //Make sure the DCC plugin zip file exists first
        DCCPluginDownloader downloader = new DCCPluginDownloader(false, 
            new string[] { dccPluginFileName }
        );
        
        string dccDesc = m_dccToolInfo.GetDescription();

        string progressBarInfo = "Installing plugin for " + dccDesc;
        EditorUtility.DisplayProgressBar("MeshSync", progressBarInfo,0);
        downloader.Execute( requestedPluginVersion, 
            /*onSuccess=*/ (string pluginVersion, List<string> dccPluginLocalPaths) => {
                EditorUtility.DisplayProgressBar("MeshSync", progressBarInfo, 0.5f);
                if (dccPluginLocalPaths.Count <= 0 || !File.Exists(dccPluginLocalPaths[0])) {
                    HandleFailedIntegration(GetLastErrorMessage(), dccDesc);
                    return;                    
                }
                
                bool dccConfigured = DCCIntegrationUtility.InstallDCCPlugin(this, m_dccToolInfo, pluginVersion, dccPluginLocalPaths[0]);
                if (!dccConfigured) {
                    HandleFailedIntegration(GetLastErrorMessage(), dccDesc);
                    return;
                }
                
                EditorUtility.ClearProgressBar();
                FinalizeDCCConfigurationV();
                
                onComplete();
            }, 
            /*onFail=*/ () => {
                Debug.LogError("[MeshSync] Failed to download DCC Plugin for " + dccDesc);
                EditorUtility.ClearProgressBar();
            }
        );
    }

//----------------------------------------------------------------------------------------------------------------------    
    internal DCCPluginInstallInfo FindInstallInfo() {
        
        string path = DCCPluginInstallInfo.GetInstallInfoPath(m_dccToolInfo);
        if (!File.Exists(path))
            return null;

        return FileUtility.DeserializeFromJson<DCCPluginInstallInfo>(path);
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    internal DCCToolInfo GetDCCToolInfo()      { return m_dccToolInfo; }
    
    internal string GetLastErrorMessage()           { return m_lastErrorMessage; }
    internal void   SetLastErrorMessage(string msg) { m_lastErrorMessage = msg; }

//----------------------------------------------------------------------------------------------------------------------    

    //The name of the DCCTool in the filename of the DCC plugin
    protected abstract string GetDCCToolInFileNameV();

    //returns null when failed
    internal abstract bool ConfigureDCCToolV( DCCToolInfo dccToolInfo, string srcPluginRoot, 
        string tempPath);
    
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
        Debug.LogError($"[MeshSync] Error when installing plugin for {dccDesc}. {System.Environment.NewLine} " +
            $"Message:  {errMessage}");
        EditorUtility.ClearProgressBar();
        EditorUtility.DisplayDialog("MeshSync",
            $"Failed in installing plugin. Please make sure to close down all running instances of {dccDesc}", 
            "Ok"
        );
        
        
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    

    private readonly DCCToolInfo m_dccToolInfo = null;
    private          string      m_lastErrorMessage = null;
}

} //end namespace