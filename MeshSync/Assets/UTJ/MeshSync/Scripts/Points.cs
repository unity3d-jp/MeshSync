using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Rendering;

namespace UTJ.MeshSync
{
    [Serializable]
    public class PointsData
    {
        public float time;
        public Vector3[] points;
        public Quaternion[] rotations;
        public Vector3[] scales;
        public Bounds bounds;

        public void Clear()
        {
            time = 0.0f;
            points = null;
            rotations = null;
            scales = null;
            bounds = default(Bounds);
        }
    }

    [ExecuteInEditMode]
    public class Points : MonoBehaviour
    {
        [SerializeField] float m_time;
        [SerializeField] List<PointsData> m_records = new List<PointsData> { new PointsData() };

        public float time
        {
            get { return m_time; }
            set { m_time = value; }
        }

        public List<PointsData> records
        {
            get { return m_records; }
        }

        public PointsData current
        {
            get
            {
                return m_records.FirstOrDefault(x => x.time >= m_time);
            }
        }
    }
}
