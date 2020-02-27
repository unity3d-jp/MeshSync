using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Rendering;

namespace Unity.MeshSync
{
    [ExecuteInEditMode]
    internal class PointCache : MonoBehaviour
    {
        [SerializeField] public Vector3[] points;
        [SerializeField] public Quaternion[] rotations;
        [SerializeField] public Vector3[] scales;
        [SerializeField] public Bounds bounds;

        public void Clear()
        {

        }
    }
}
