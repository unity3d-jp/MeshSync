using System;
using System.IO;
using Unity.AnimeToolbox;
using Unity.SharpZipLib.Utils;
using UnityEngine;

namespace UnityEditor.MeshSync {

internal class MayaIntegrator : BaseDCCIntegrator {
    protected override string GetDCCName() {
        return "Maya";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override string IntegrateInternal(string localPluginPath) {

        string tempPath = FileUtil.GetUniqueTempPathInProject();
        Directory.CreateDirectory(tempPath);
        ZipUtility.UncompressFromZip(localPluginPath, null, tempPath);

        string srcFolder = Path.Combine(tempPath, Path.GetFileNameWithoutExtension(localPluginPath));
        if (!Directory.Exists(srcFolder)) {
            Debug.LogError("[MeshSync] Failed to install DCC Plugin for Maya");
            return null;
        }

        string integrationRootFolder = null;
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                // If MAYA_APP_DIR environment variable is setup, copy the modules directory there.
                //     If not, go to %USERPROFILE%\Documents\maya in Windows Explorer, and copy the modules directory there.
                break;
            }
            case RuntimePlatform.OSXEditor: {
                integrationRootFolder = "/Users/Shared/Autodesk/modules/maya";
                FileUtility.CopyRecursive(srcFolder, integrationRootFolder,true);
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
        FileUtility.DeleteFilesAndFolders(tempPath);
        
        return integrationRootFolder;
    }
    
}
} // end namespace
