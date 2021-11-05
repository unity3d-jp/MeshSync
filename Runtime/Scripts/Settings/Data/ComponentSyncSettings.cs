using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class ComponentSyncSettings {
    
    [SerializeField] internal bool Create;
    [SerializeField] internal bool Update;
    
}

} //end namespace