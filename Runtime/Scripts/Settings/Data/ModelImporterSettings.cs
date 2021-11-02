using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal struct ModelImporterSettings {
    
    internal ModelImporterSettings(ModelImporterSettings other) {
        this.CreateMaterials = other.CreateMaterials;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] internal bool CreateMaterials;
}

} //end namespace