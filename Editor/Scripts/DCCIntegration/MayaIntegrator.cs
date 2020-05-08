using System;

namespace UnityEditor.MeshSync {

internal class MayaIntegrator : BaseDCCIntegrator {
    protected override string GetDCCName() {
        return "Maya";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void IntegrateInternal() {
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
        
        throw new NotImplementedException();
    }
    
}
} // end namespace
