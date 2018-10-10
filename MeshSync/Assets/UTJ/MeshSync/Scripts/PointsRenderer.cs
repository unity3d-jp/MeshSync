using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace UTJ.MeshSync
{
    [ExecuteInEditMode]
    [RequireComponent(typeof(Points))]
    public class PointsRenderer : MonoBehaviour
    {
        [SerializeField] Mesh m_mesh;
        [SerializeField] Material[] m_materials;
        [SerializeField] ShadowCastingMode m_castShadows = ShadowCastingMode.On;
        [SerializeField] bool m_applyTransform = true;
        [SerializeField] bool m_receiveShadows = true;
        [SerializeField] float m_pointSize = 0.2f;

        Mesh m_prevMesh;
        ComputeBuffer m_cbPoints;
        ComputeBuffer m_cbRotations;
        ComputeBuffer m_cbScales;
        ComputeBuffer m_cbColors;
        ComputeBuffer[] m_cbArgs;
        int[] m_args = new int[5] { 0, 0, 0, 0, 0 };
        MaterialPropertyBlock m_mpb;



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


        public void Flush(PointsData data)
        {
            if (data == null || data.points.Length == 0)
                return;
            if (m_mesh == null || m_materials == null)
                return;

            int numInstances = data.points.Length;
            int submeshCount = System.Math.Min(m_mesh.subMeshCount, m_materials.Length);
            int layer = gameObject.layer;

            bool supportsInstancing = SystemInfo.supportsInstancing && SystemInfo.supportsComputeShaders;
            if (!supportsInstancing)
            {
                Debug.LogWarning("PointsRenderer: Instancing is not supported on this system.");
                return;
            }

            // check if mesh changed
            if (m_prevMesh != m_mesh)
            {
                m_prevMesh = m_mesh;
                if (m_cbArgs != null)
                {
                    foreach (var cb in m_cbArgs) { cb.Release(); }
                    m_cbArgs = null;
                }
            }


            // update buffers
            if (m_cbPoints != null && m_cbPoints.count < numInstances)
            {
                m_cbPoints.Release();
                m_cbPoints = null;
            }
            if (m_cbPoints == null)
                m_cbPoints = new ComputeBuffer(numInstances, 12);

            if (m_cbRotations != null && m_cbRotations.count < numInstances)
            {
                m_cbRotations.Release();
                m_cbRotations = null;
            }
            if (m_cbRotations == null)
                m_cbRotations = new ComputeBuffer(numInstances, 16);

            if (m_cbScales != null && m_cbScales.count < numInstances)
            {
                m_cbScales.Release();
                m_cbScales = null;
            }
            if (m_cbScales == null)
                m_cbScales = new ComputeBuffer(numInstances, 12);

            if (m_cbColors != null && m_cbColors.count < numInstances)
            {
                m_cbColors.Release();
                m_cbColors = null;
            }
            if (m_cbColors == null)
                m_cbColors = new ComputeBuffer(numInstances, 16);

            m_cbPoints.SetData(data.points);
            m_cbRotations.SetData(data.rotations);
            m_cbScales.SetData(data.scales);
            m_cbColors.SetData(data.colors);


            var trans = GetComponent<Transform>();
            var pos = trans.position;
            var rot = trans.rotation;
            var scl = trans.lossyScale;

            // update materials
            if (m_mpb == null)
                m_mpb = new MaterialPropertyBlock();
            if (m_applyTransform)
            {
                m_mpb.SetVector("_Position", pos);
                m_mpb.SetVector("_Rotation", new Vector4(rot.x, rot.y, rot.z, rot.w));
                m_mpb.SetVector("_Scale", scl);
            }
            else
            {
                m_mpb.SetVector("_Position", Vector3.zero);
                m_mpb.SetVector("_Rotation", new Vector4(0, 0, 0, 1));
                m_mpb.SetVector("_Scale", Vector3.one);
            }
            m_mpb.SetFloat("_PointSize", m_pointSize);
            m_mpb.SetBuffer("_Points", m_cbPoints);
            m_mpb.SetBuffer("_Rotations", m_cbRotations);
            m_mpb.SetBuffer("_Scales", m_cbScales);
            m_mpb.SetBuffer("_Colors", m_cbColors);

            // update argument buffer
            if (m_cbArgs == null || m_cbArgs.Length != submeshCount)
            {
                if (m_cbArgs != null)
                {
                    foreach (var cb in m_cbArgs)
                        cb.Release();
                    m_cbArgs = null;
                }

                m_cbArgs = new ComputeBuffer[submeshCount];
                for (int si = 0; si < submeshCount; ++si)
                    m_cbArgs[si] = new ComputeBuffer(1, m_args.Length * sizeof(uint), ComputeBufferType.IndirectArguments);
            }
            for (int si = 0; si < submeshCount; ++si)
            {
                m_args[0] = (int)m_mesh.GetIndexCount(si);
                m_args[1] = numInstances;
                m_cbArgs[si].SetData(m_args);
            }

            // issue drawcalls
            int n = Math.Min(submeshCount, m_materials.Length);
            for (int si = 0; si < n; ++si)
            {
                var args = m_cbArgs[si];
                var material = m_materials[si];
                if (material == null)
                    continue;
                Graphics.DrawMeshInstancedIndirect(m_mesh, si, material,
                    data.bounds, args, 0, m_mpb, m_castShadows, m_receiveShadows, layer);
            }
        }

        public void Release()
        {
            if (m_cbArgs != null)
            {
                foreach (var cb in m_cbArgs) { cb.Release(); }
                m_cbArgs = null;
            }
            if (m_cbPoints != null) { m_cbPoints.Release(); m_cbPoints = null; }
            if (m_cbRotations != null) { m_cbRotations.Release(); m_cbRotations = null; }
            if (m_cbScales != null) { m_cbScales.Release(); m_cbScales = null; }
            if (m_cbColors != null) { m_cbColors.Release(); m_cbColors = null; }
        }


        void OnDisable()
        {
            Release();
        }

        void LateUpdate()
        {
            var points = GetComponent<Points>();
            Flush(points.current);
        }
    }
}
