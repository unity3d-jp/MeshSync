using System;


namespace UnityEditor.MeshSync {

internal class DCCIntegratorFactory {

    internal static BaseDCCIntegrator Create(DCCToolType dccToolType) {

        switch (dccToolType) {
            case DCCToolType.AUTODESK_MAYA: return new MayaIntegrator();
            case DCCToolType.AUTODESK_3DSMAX: return new _3DSMaxIntegrator();
            default: throw new NotImplementedException();
        }
    }
    
}

} //end namespace
