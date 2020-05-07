using System;


namespace UnityEditor.MeshSync {
    
internal static class DCCIntegrationUtility {

    internal static bool InstallPlugin(DCCToolInfo dccToolInfo) {

        switch (dccToolInfo.Type) {
            case DCCToolType.AUTODESK_MAYA: {
                return InstallMayaPlugin();
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

    static bool InstallMayaPlugin() {
        
        //[TODO-sin: 2020-5-7] Implement this
        //Check file if it exists. if it doesn't, try to download it
            
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

} //end namespace