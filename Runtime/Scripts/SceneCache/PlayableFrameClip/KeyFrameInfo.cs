using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal struct KeyFrameInfo {
    [SerializeField] internal bool         enabled;
    [SerializeField] internal int          frameNo;
    [SerializeField] internal KeyFrameMode mode;
}

} //end namespace
