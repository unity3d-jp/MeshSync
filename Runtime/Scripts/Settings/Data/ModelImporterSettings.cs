using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class ModelImporterSettings {

    internal ModelImporterSettings() {}

    internal ModelImporterSettings(ModelImporterSettings other) {
        m_syncMaterials = other.m_syncMaterials;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private bool m_syncMaterials = true;    
}

} //end namespace