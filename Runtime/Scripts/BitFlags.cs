using System;
using UnityEngine;

namespace Unity.MeshSync
{
    [Serializable]
    internal struct BitFlags
    {
        // int because uint is not serialized...
        [SerializeField] public int bits;

        public bool this[int v]
        {
            get
            {
                return (bits & (1 << v)) != 0;
            }
            set
            {
                if (value)
                    bits |= (1 << v);
                else
                    bits &= ~(1 << v);
            }
        }
    }
}
