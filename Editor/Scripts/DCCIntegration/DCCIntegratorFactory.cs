using System;


namespace Unity.MeshSync.Editor {

internal class DCCIntegratorFactory {

    internal static BaseDCCIntegrator Create(DCCToolInfo dccToolInfo) {

        switch (dccToolInfo.Type) {
            case DCCToolType.AUTODESK_MAYA: return new MayaIntegrator(dccToolInfo);
            case DCCToolType.AUTODESK_3DSMAX: return new _3DSMaxIntegrator(dccToolInfo);
            case DCCToolType.BLENDER: return new BlenderIntegrator(dccToolInfo);
            default: throw new NotImplementedException();
        }
    }
    
}

} //end namespace
