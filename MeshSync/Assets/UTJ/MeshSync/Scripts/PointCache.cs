using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Rendering;

namespace UTJ.MeshSync
{
    [ExecuteInEditMode]
    public class PointCache : MonoBehaviour
    {
        [Serializable]
        public class Data
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

        [SerializeField] float m_time;
        [SerializeField] List<Data> m_data = new List<Data> { new Data() };

        public float time
        {
            get { return m_time; }
            set { m_time = value; }
        }

        public List<Data> data
        {
            get { return m_data; }
        }

        public Data current
        {
            get
            {
                if (m_time <= m_data[0].time)
                    return m_data[0];
                else if (m_time >= m_data[m_data.Count - 1].time)
                    return m_data[m_data.Count - 1];
                else
                    return m_data.FirstOrDefault(x => x.time >= m_time);
            }
        }
    }
}
