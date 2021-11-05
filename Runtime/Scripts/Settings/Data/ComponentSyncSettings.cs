using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class ComponentSyncSettings {

    internal ComponentSyncSettings() {
        
    }

    internal ComponentSyncSettings(ComponentSyncSettings other) {
        this.Create = other.Create;
        this.Update = other.Update;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] internal bool Create;
    [SerializeField] internal bool Update;
    
}

} //end namespace