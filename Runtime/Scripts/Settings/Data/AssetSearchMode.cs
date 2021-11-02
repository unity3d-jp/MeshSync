using UnityEngine;

namespace Unity.MeshSync {

internal enum AssetSearchMode {

    [InspectorName("Local")] LOCAL = 0,
    [InspectorName("Recursive Up")] RECURSIVE_UP,
    [InspectorName("Everywhere")] EVERYWHERE,
    
}

} //end namespace