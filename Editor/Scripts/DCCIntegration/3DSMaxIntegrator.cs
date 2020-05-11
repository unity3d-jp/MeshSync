using System;
using System.IO;
using Unity.AnimeToolbox;
using Unity.SharpZipLib.Utils;
using UnityEngine;

namespace UnityEditor.MeshSync {
internal class _3DSMaxIntegrator : BaseDCCIntegrator {
    internal _3DSMaxIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileName() {
        return "3DSMAX";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override DCCPluginInstallInfo ConfigureDCCTool(DCCToolInfo dccToolInfo, string configFolder, 
        string localPluginPath) 
    {
        //[TODO-sin: 2020-5-7] Implement this
        //Copy the file to The plugin path under the installation directory,
        //e.g: C:\Program Files\Autodesk\3ds Max 2019\Plugins            
        

        string tempPath = FileUtil.GetUniqueTempPathInProject();
        
        Directory.CreateDirectory(tempPath);
        ZipUtility.UncompressFromZip(localPluginPath, null, tempPath);
        
        
        //Cleanup
        FileUtility.DeleteFilesAndFolders(tempPath);
        
        return new DCCPluginInstallInfo(dccToolInfo.DCCToolVersion);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    protected override string FindConfigFolder() {
        //Sample: "C:\Program Files\Autodesk\3ds Max 2019\Plugins"
        DCCToolInfo dccToolInfo = GetDCCToolInfo();
        return Path.GetDirectoryName(dccToolInfo.AppPath) + @"\Plugins";
    }
    
}

} //end namespace
