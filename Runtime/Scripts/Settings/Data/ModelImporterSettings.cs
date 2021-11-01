using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class ModelImporterSettings {

    internal ModelImporterSettings() { }
    
    internal ModelImporterSettings(ModelImporterSettings other) {
        this.CreateMaterials = other.CreateMaterials;
    }
    
    internal ModelImporterSettings(bool createMaterials) {
        this.CreateMaterials = createMaterials;
    }
    

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] internal bool CreateMaterials;
}

} //end namespace