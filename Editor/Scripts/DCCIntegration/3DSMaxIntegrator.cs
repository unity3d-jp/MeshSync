using System.IO;

namespace UnityEditor.MeshSync {
internal class _3DSMaxIntegrator : BaseDCCIntegrator {
    internal _3DSMaxIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "3DSMAX";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool ConfigureDCCToolV(DCCToolInfo dccToolInfo, string pluginFileName, string extractedTempPath) 
    {        
        //[TODO-sin: 2020-5-7] Implement this
        //Copy the file to The plugin path under the installation directory,
        //e.g: C:\Program Files\Autodesk\3ds Max 2019\Plugins            
        
        
        return false;
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void FinalizeDCCConfigurationV() {
        //[TODO-sin: 2020-5-12] Implement this
    }
    
//----------------------------------------------------------------------------------------------------------------------
    private string FindConfigFolder() {
        //Sample: "C:\Program Files\Autodesk\3ds Max 2019\Plugins"
        DCCToolInfo dccToolInfo = GetDCCToolInfo();
        return Path.GetDirectoryName(dccToolInfo.AppPath) + @"\Plugins";
    }
    
}

} //end namespace
