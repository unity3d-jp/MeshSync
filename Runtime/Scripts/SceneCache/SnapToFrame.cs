using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal enum SnapToFrame {
    [InspectorName("None")] NONE,
    [InspectorName("Nearest")] NEAREST,
}

} //namespace