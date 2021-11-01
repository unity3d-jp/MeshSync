using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal struct ModelImporterSettings {

    internal ModelImporterSettings(ModelImporterSettings other) {
        m_syncMaterials = other.m_syncMaterials;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private bool m_syncMaterials;
}

} //end namespace