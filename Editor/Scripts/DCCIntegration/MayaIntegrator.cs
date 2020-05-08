using System;
using System.IO;
using Unity.AnimeToolbox;
using Unity.SharpZipLibUnity.Utils;
using UnityEngine;

namespace UnityEditor.MeshSync {

internal class MayaIntegrator : BaseDCCIntegrator {
    protected override string GetDCCName() {
        return "Maya";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void IntegrateInternal(string localPluginPath) {

        /*
        string tempPath = FileUtil.GetUniqueTempPathInProject();
        Directory.CreateDirectory(tempPath);
        
        ZipUtility.UncompressFromZip(pluginPath, null, tempPath);
        */

        
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                // If MAYA_APP_DIR environment variable is setup, copy the modules directory there.
                //     If not, go to %USERPROFILE%\Documents\maya in Windows Explorer, and copy the modules directory there.
                break;
            }
            case RuntimePlatform.OSXEditor: {
                try {
                    Debug.Log(localPluginPath);
                    ZipUtility.UncompressFromZip(localPluginPath, null, "/Users/Shared/Autodesk/modules/maya");

                }
                catch {
                    Debug.LogError("FAIL");
                }
                
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                // Copy the modules directory to ~/maya/<maya_version)            
                break;
            }
            default: {
                throw new NotImplementedException();
            }
        }
        

        //Cleanup
        //FileUtility.DeleteFilesAndFolders(tempPath);
    }
    
}
} // end namespace
