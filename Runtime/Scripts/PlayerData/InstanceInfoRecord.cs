using System;
using UnityEngine;

namespace Unity.MeshSync
{
    [Serializable]
    internal class InstanceInfoRecord
    {
        public GameObject go;
        public MeshSyncInstanceRenderer renderer;
    }
}