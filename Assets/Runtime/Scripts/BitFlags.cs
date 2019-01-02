using System;
using UnityEngine;

namespace UTJ.MeshSync
{
    [Serializable]
    public struct BitFlags
    {
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
