using System;
using UnityEngine;

namespace Unity.MeshSync {

//[Note-sin: 2021-9-9] Don't remove any previous enum values
[Serializable]
internal enum SnapToFrame {
    [InspectorName("None")] NONE,
    [InspectorName("Nearest")] NEAREST,
}

} //namespace