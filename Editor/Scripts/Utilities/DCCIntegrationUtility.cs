using System;
using System.IO;
using UnityEngine;


namespace UnityEditor.MeshSync {
    
internal static class DCCIntegrationUtility {

    internal static void InstallPlugin(DCCToolInfo dccToolInfo) {

        switch (dccToolInfo.Type) {
            case DCCToolType.AUTODESK_MAYA: {
                InstallMayaPlugin();
                break;
            }
            case DCCToolType.AUTODESK_3DSMAX: {
                throw new NotImplementedException();
            }
            default: {
                throw new NotImplementedException();
            }
        }
    }
    
//----------------------------------------------------------------------------------------------------------------------

    static void InstallMayaPlugin() {
        
        const string PLUGIN_NAME = "Maya";
        string dccPlatformName = PLUGIN_NAME + "_" + GetCurrentDCCPluginPlatform() + ".zip";
        
        //Make sure the file exists first
        DCCPluginDownloader downloader = new DCCPluginDownloader(false,SAVED_PLUGINS_FOLDER, 
            new string[] { dccPlatformName }
        );
        
        EditorUtility.DisplayProgressBar("MeshSync", "Installing plugin for " + PLUGIN_NAME,0);
        downloader.Execute(() => {
            Debug.Log("File copied to: " + SAVED_PLUGINS_FOLDER);
            
            //[TODO-sin: 2020-5-7] Implement this
            //Copy the file to 
            // Windows:
            // If MAYA_APP_DIR environment variable is setup, copy the modules directory there.
            //     If not, go to %USERPROFILE%\Documents\maya in Windows Explorer, and copy the modules directory there.
            //     Mac:
            // Copy the UnityMeshSync directory and UnityMeshSync.mod file to /Users/Shared/Autodesk/modules/maya.
            //     Linux:
            // Copy the modules directory to ~/maya/<maya_version)            
            //Set configuration
            
            EditorUtility.ClearProgressBar();
        }, () => {
            Debug.LogError("Failed to download DCC Plugin for " + PLUGIN_NAME);
            EditorUtility.ClearProgressBar();
        });
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