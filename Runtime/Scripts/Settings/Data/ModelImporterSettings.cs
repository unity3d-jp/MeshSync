using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class ModelImporterSettings {

    internal ModelImporterSettings() { }
    
    internal ModelImporterSettings(ModelImporterSettings other) {
        this.SyncMaterials = other.SyncMaterials;
    }
    
    internal ModelImporterSettings(bool syncMaterials) {
        this.SyncMaterials = syncMaterials;
    }
    

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] internal bool SyncMaterials;
}

} //end namespace