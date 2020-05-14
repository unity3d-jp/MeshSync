using System.IO;
using Unity.AnimeToolbox;
using Unity.SharpZipLib.Utils;

namespace UnityEditor.MeshSync {
internal class BlenderIntegrator : BaseDCCIntegrator {
    internal BlenderIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "Blender";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool  ConfigureDCCToolV(DCCToolInfo dccToolInfo, string localPluginPath) 
    {
        //[TODO-sin: 2020-5-7] Implement this
        //Execute the following
        //Might be different for different versions of Blender
        // import bpy
        // bpy.ops.preferences.addon_install(overwrite=True, filepath="/Users/shin/G/UT/MeshSyncDCCPlugin/Plugins~/Dist/UnityMeshSync_0.1.0-preview_Blender_Mac/blender-2.80.zip")
        // bpy.ops.preferences.addon_enable(module='unity_mesh_sync')
        // bpy.ops.wm.save_userpref()
        
        

        string tempPath = FileUtil.GetUniqueTempPathInProject();
        
        Directory.CreateDirectory(tempPath);
        ZipUtility.UncompressFromZip(localPluginPath, null, tempPath);
        
        //Cleanup
        FileUtility.DeleteFilesAndFolders(tempPath);
        
        return false;
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void FinalizeDCCConfigurationV() {
        //[TODO-sin: 2020-5-12] Implement this
    }
    

    
}

} //end namespace
