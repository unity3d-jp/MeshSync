using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace UTJ.MeshSync
{
    [ExecuteInEditMode]
    [RequireComponent(typeof(PointCache))]
    public class PointCacheRenderer : MonoBehaviour
    {
        #region Fields
        [SerializeField] Mesh m_mesh;
        [SerializeField] Material[] m_materials;
        [SerializeField] float m_pointSize = 1.0f;
        [SerializeField] bool m_applyTransform = true;
        [SerializeField] ShadowCastingMode m_castShadows = ShadowCastingMode.On;
        [SerializeField] bool m_receiveShadows = true;
        [SerializeField] int m_batchSize = 1024;
        MaterialPropertyBlock m_mpb;
        Matrix4x4[] m_matrices;
        #endregion

        #region Properties
        public Mesh sharedMesh
        {
            get { return m_mesh; }
            set { m_mesh = value; }
        }
        public Material[] sharedMaterials
        {
            get { return m_materials; }
            set { m_materials = value; }
        }
        public float pointSize
        {
            get { return m_pointSize; }
            set { m_pointSize = value; }
        }
        public bool applyTransform
        {
            get { return m_applyTransform; }
            set { m_applyTransform = value; }
        }
        public ShadowCastingMode castShadows
        {
            get { return m_castShadows; }
            set { m_castShadows = value; }
        }
        public bool receiveShadows
        {
            get { return m_receiveShadows; }
            set { m_receiveShadows = value; }
        }
        public int batchSize
        {
            get { return m_batchSize; }
            set { m_batchSize = value; OnValidate(); }
        }
        #endregion


        #region Impl
        bool IsValidArray<T>(T[] a)
        {
            return a != null && a.Length > 0;
        }

        public void Flush(PointCache.Data data)
        {
            if (data == null || data.points == null || data.points.Length == 0)
                return;
            if (m_mesh == null || m_materials == null)
                return;
            if (!SystemInfo.supportsInstancing)
            {
                Debug.LogWarning("PointsRenderer: Instancing is not supported on this system.");
                return;
            }

            if (m_mpb == null)
                m_mpb = new MaterialPropertyBlock();

            int submeshCount = Math.Min(m_mesh.subMeshCount, m_materials.Length);
            int layer = gameObject.layer;
            int numInstances = data.points.Length;
            int numBatches = numInstances / m_batchSize + (numInstances % m_batchSize > 0 ? 1 : 0);

            bool hasPoints = IsValidArray(data.points);
            bool hasRotations = IsValidArray(data.rotations);
            bool hasScales = IsValidArray(data.scales);

            var identity = Quaternion.identity;
            var zero = Vector3.zero;
            var one = Vector3.one;

            Matrix4x4 local = m_applyTransform ? GetComponent<Transform>().localToWorldMatrix : Matrix4x4.identity;
            if(m_matrices == null || m_matrices.Length != m_batchSize)
                m_matrices = new Matrix4x4[m_batchSize];

            for (int bi = 0; bi < numBatches; ++bi)
            {
                int batchBegin = m_batchSize * bi;
                int n = Math.Min(numInstances - batchBegin, m_batchSize);

                for (int ni = 0; ni < n; ++ni)
                {
                    int ii = batchBegin + ni;
                    m_matrices[ni] = local * Matrix4x4.TRS(
                        hasPoints ? data.points[ii] : zero,
                        hasRotations ? data.rotations[ii] : identity,
                        (hasScales ? data.scales[ii] : one) * m_pointSize);
                }
                for (int si = 0; si < submeshCount; ++si)
                {
                    var material = m_materials[si];
                    if (material == null)
                        continue;
                    Graphics.DrawMeshInstanced(m_mesh, si, material, m_matrices, n, m_mpb, m_castShadows, m_receiveShadows, layer);
                }
            }
        }

        void OnValidate()
        {
            m_batchSize = Mathf.Clamp(m_batchSize, 1, 1024);
        }

        void LateUpdate()
        {
            var points = GetComponent<PointCache>();
            Flush(points.current);
        }
        #endregion
    }
}
