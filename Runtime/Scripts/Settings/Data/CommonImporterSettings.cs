using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class CommonImporterSettings {

    internal CommonImporterSettings() {}

    internal CommonImporterSettings(CommonImporterSettings other) {
        m_syncMaterials = other.m_syncMaterials;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private bool m_syncMaterials = true;    
}

} //end namespace