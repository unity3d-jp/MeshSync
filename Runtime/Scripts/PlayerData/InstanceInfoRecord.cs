using System;
using UnityEngine;

namespace Unity.MeshSync
{
    [Serializable]
    internal class InstanceInfoRecord
    {
        public GameObject go;
        public Matrix4x4[] transforms;
    }
}