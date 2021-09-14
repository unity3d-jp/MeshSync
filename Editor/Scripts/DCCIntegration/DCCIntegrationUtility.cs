using System;
using System.IO;
using Unity.FilmInternalUtilities;
using Unity.SharpZipLib.Utils;
using UnityEditor;

namespace Unity.MeshSync.Editor {

[Serializable]
internal static class DCCIntegrationUtility {
    
    internal static bool InstallDCCPlugin(BaseDCCIntegrator integrator, DCCToolInfo  dccToolInfo, string pluginVersion, string dccPluginLocalPath) {
        bool dccConfigured = false;
            
        //Extract
        string localPluginPath = dccPluginLocalPath;
        string tempPath        = FileUtil.GetUniqueTempPathInProject();        
        Directory.CreateDirectory(tempPath);
        ZipUtility.UncompressFromZip(localPluginPath, null, tempPath);

        //Go down one folder
        string[] extractedDirs = Directory.GetDirectories(tempPath);
        if (extractedDirs.Length > 0) {
            dccConfigured = integrator.ConfigureDCCToolV(dccToolInfo, extractedDirs[0],tempPath);
        } 
        
        //Cleanup
        FileUtility.DeleteFilesAndFolders(tempPath);
        
        if (!dccConfigured) {
            return false;
        }

        string installInfoPath   = DCCPluginInstallInfo.GetInstallInfoPath(dccToolInfo);
        string installInfoFolder = Path.GetDirectoryName(installInfoPath);
        if (null == installInfoPath || null == installInfoFolder) {
            return false;
        }

        //Write DCCPluginInstallInfo for the version
        Directory.CreateDirectory(installInfoFolder);

        DCCPluginInstallInfo installInfo =  FileUtility.DeserializeFromJson<DCCPluginInstallInfo>(installInfoPath);
        if (null == installInfo) {
            installInfo = new DCCPluginInstallInfo();
        }
        installInfo.SetPluginVersion(dccToolInfo.AppPath, pluginVersion);

        try {
            FileUtility.SerializeToJson(installInfo, installInfoPath);
        } catch (Exception e) {
            integrator.SetLastErrorMessage(e.ToString());
            return false;
        }

        return true;        
    }

}

} //end namespace