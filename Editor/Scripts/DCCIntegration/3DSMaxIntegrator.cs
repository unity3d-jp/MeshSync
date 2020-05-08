using System;

namespace UnityEditor.MeshSync {
internal class _3DSMaxIntegrator : BaseDCCIntegrator {
    internal _3DSMaxIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolName() {
        return "3DSMAX";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override DCCPluginInstallInfo ConfigureDCCTool(DCCToolInfo dccToolInfo, string configFolder, 
        string localPluginPath) 
    {
        //[TODO-sin: 2020-5-7] Implement this
        //Copy the file to The plugin path under the installation directory,
        //e.g: C:\Program Files\Autodesk\3ds Max 2019\Plugins            
        
        throw new NotImplementedException();
    }
    
//----------------------------------------------------------------------------------------------------------------------
    protected override string FindConfigFolder() {
        throw new NotImplementedException();
    }
    
}

} //end namespace
