using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;

namespace UnityEditor.MeshSync {

internal abstract class BaseDCCIntegrator {

    internal void Integrate() {
        string dccPluginFileName = GetDCCPluginFileName();
    
        //Make sure the DCC plugin zip file exists first
        DCCPluginDownloader downloader = new DCCPluginDownloader(false,SAVED_PLUGINS_FOLDER, 
            new string[] { dccPluginFileName }
        );

        string dccName = GetDCCName();
        EditorUtility.DisplayProgressBar("MeshSync", "Installing plugin for " + dccName,0);
        downloader.Execute((string version, List<string> dccPluginLocalPaths) => {

            if (dccPluginLocalPaths.Count <= 0 || !File.Exists(dccPluginLocalPaths[0])) {
                Debug.LogError("[MeshSync] Unknown error when installing plugin for " + dccName);
            } else {
                IntegrateInternal(dccPluginLocalPaths[0]);
            }
            
            //Write metafile for the version
        
            EditorUtility.ClearProgressBar();
        }, () => {
            Debug.LogError("Failed to download DCC Plugin for " + dccName);
            EditorUtility.ClearProgressBar();
        });
        
    }
//----------------------------------------------------------------------------------------------------------------------    

    protected abstract string GetDCCName();
    protected abstract void IntegrateInternal(string localPluginPath);


//----------------------------------------------------------------------------------------------------------------------    

    private string GetDCCPluginFileName() {
        return GetDCCName() + "_" + GetCurrentDCCPluginPlatform() + ".zip";
        
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

    private static readonly string SAVED_PLUGINS_FOLDER = Path.Combine(Application.dataPath, "MeshSyncDCCPlugins~");
    
}

} //end namespace