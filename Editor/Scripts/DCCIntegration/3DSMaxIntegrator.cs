using System;

namespace UnityEditor.MeshSync {
internal class _3DSMaxIntegrator : BaseDCCIntegrator {
    protected override string GetDCCName() {
        return "3DSMAX";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void IntegrateInternal(string localPluginPath) {
        //[TODO-sin: 2020-5-7] Implement this
        //Copy the file to The plugin path under the installation directory,
        //e.g: C:\Program Files\Autodesk\3ds Max 2019\Plugins            
        
        throw new NotImplementedException();
    }

}

} //end namespace
