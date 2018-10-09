using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace UTJ.MeshSync
{
    public static class VPMatrices
    {
        static Dictionary<Camera, Matrix4x4> m_currentVPMatrix = new Dictionary<Camera, Matrix4x4>();
        static Dictionary<Camera, Matrix4x4> m_previousVPMatrix = new Dictionary<Camera, Matrix4x4>();
        static int m_frameCount;

        public static Matrix4x4 Get(Camera camera)
        {
            if (Time.frameCount != m_frameCount) SwapMatrixMap();

            Matrix4x4 m;
            if (!m_currentVPMatrix.TryGetValue(camera, out m))
            {
                m = camera.nonJitteredProjectionMatrix * camera.worldToCameraMatrix;
                m_currentVPMatrix.Add(camera, m);
            }

            return m;
        }

        public static Matrix4x4 GetPrevious(Camera camera)
        {
            if (Time.frameCount != m_frameCount) SwapMatrixMap();

            Matrix4x4 m;
            if (m_previousVPMatrix.TryGetValue(camera, out m))
                return m;
            else
                return Get(camera);
        }

        static void SwapMatrixMap()
        {
            var temp = m_previousVPMatrix;
            m_previousVPMatrix = m_currentVPMatrix;
            temp.Clear();
            m_currentVPMatrix = temp;
            m_frameCount = Time.frameCount;
        }
    }

    public class PointsRenderer : MonoBehaviour
    {
        [SerializeField] Mesh m_mesh;
        [SerializeField] Material[] m_materials;
        [SerializeField] Material m_motionVectorMaterial;
        [SerializeField] ShadowCastingMode m_castShadows = ShadowCastingMode.On;
        [SerializeField] bool m_applyTransform = true;
        [SerializeField] bool m_receiveShadows = true;
        [SerializeField] bool m_generateMotionVector = true;
        [SerializeField] float m_pointSize = 0.2f;

        Mesh m_prevMesh;
        ComputeBuffer m_cbPoints;
        ComputeBuffer m_cbRotations;
        ComputeBuffer m_cbScales;
        ComputeBuffer m_cbColors;
        ComputeBuffer[] m_cbArgs;
        CommandBuffer m_cmdMotionVector;
        int[] m_args = new int[5] { 0, 0, 0, 0, 0 };
        Bounds m_bounds;
        MaterialPropertyBlock m_mpb;

        Vector3 m_position, m_positionOld;
        Quaternion m_rotation, m_rotationOld;
        Vector3 m_scale, m_scaleOld;

        PinnedList<Vector3> m_points = new PinnedList<Vector3>();
        PinnedList<Quaternion> m_rotations = new PinnedList<Quaternion>();
        PinnedList<Vector3> m_scales = new PinnedList<Vector3>();
        PinnedList<Color> m_colors = new PinnedList<Color>();

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

        public Material motionVectorMaterial
        {
            get { return m_motionVectorMaterial; }
            set { m_motionVectorMaterial = value; }
        }


        public void Flush()
        {
            int numInstances = m_points.Count;
            if (numInstances == 0) { return; }

            var materials = m_materials;
            var mesh = m_mesh;
            if (mesh == null || materials == null) { return; }

            int submeshCount = System.Math.Min(mesh.subMeshCount, materials.Length);
            int layer = gameObject.layer;

            bool supportsInstancing = SystemInfo.supportsInstancing && SystemInfo.supportsComputeShaders;
            if (!supportsInstancing)
            {
                Debug.LogWarning("PointsRenderer: Instancing is not supported on this system.");
                return;
            }

            // check if mesh changed
            if (m_prevMesh != mesh)
            {
                m_prevMesh = mesh;
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

#if UNITY_2017_3_OR_NEWER
            m_cbPoints.SetData(m_points.List);
            m_cbRotations.SetData(m_rotations.List);
            m_cbScales.SetData(m_scales.List);
            m_cbColors.SetData(m_colors.List);
#else
            m_cbPoints.SetData(m_points.Array);
            m_cbRotations.SetData(m_rotations.Array);
            m_cbScales.SetData(m_scales.Array);
            m_cbColors.SetData(m_colors.Array);
#endif


            // build bounds
            // todo:
            //m_bounds = new Bounds(apc.m_boundsCenter, apc.m_boundsExtents + mesh.bounds.extents);


            // update materials
            if (m_mpb == null)
                m_mpb = new MaterialPropertyBlock();
            if (m_applyTransform)
            {
                m_mpb.SetVector("_Position", m_position);
                m_mpb.SetVector("_PositionOld", m_positionOld);
                m_mpb.SetVector("_Rotation", new Vector4(m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w));
                m_mpb.SetVector("_RotationOld", new Vector4(m_rotationOld.x, m_rotationOld.y, m_rotationOld.z, m_rotationOld.w));
                m_mpb.SetVector("_Scale", m_scale);
                m_mpb.SetVector("_ScaleOld", m_scaleOld);
            }
            else
            {
                m_mpb.SetVector("_Position", Vector3.zero);
                m_mpb.SetVector("_PositionOld", Vector3.zero);
                m_mpb.SetVector("_Rotation", new Vector4(0, 0, 0, 1));
                m_mpb.SetVector("_RotationOld", new Vector4(0, 0, 0, 1));
                m_mpb.SetVector("_Scale", Vector3.one);
                m_mpb.SetVector("_ScaleOld", Vector3.one);
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
                m_args[0] = (int)mesh.GetIndexCount(si);
                m_args[1] = numInstances;
                m_cbArgs[si].SetData(m_args);
            }

            // issue drawcalls
            int n = Math.Min(submeshCount, materials.Length);
            for (int si = 0; si < n; ++si)
            {
                var args = m_cbArgs[si];
                var material = materials[si];
                if (material == null)
                    continue;
                Graphics.DrawMeshInstancedIndirect(mesh, si, material,
                    m_bounds, args, 0, m_mpb, m_castShadows, m_receiveShadows, layer);
            }
        }

        void FlushMotionVector()
        {
            if (!m_generateMotionVector || Camera.current == null || (Camera.current.depthTextureMode & DepthTextureMode.MotionVectors) == 0)
                return;

            // assume setup is already done in Flush()

            var material = m_motionVectorMaterial;
            var mesh = m_mesh;
            if (mesh == null || material == null)
                return;

            material.SetMatrix("_PreviousVP", VPMatrices.GetPrevious(Camera.current));
            material.SetMatrix("_NonJitteredVP", VPMatrices.Get(Camera.current));

            int layer = gameObject.layer;

            if (m_cmdMotionVector == null)
            {
                m_cmdMotionVector = new CommandBuffer();
                m_cmdMotionVector.name = "MeshSync PointsRenderer";
            }
            m_cmdMotionVector.Clear();
            m_cmdMotionVector.SetRenderTarget(BuiltinRenderTextureType.MotionVectors, BuiltinRenderTextureType.CameraTarget);
            for (int si = 0; si < mesh.subMeshCount; ++si)
                m_cmdMotionVector.DrawMeshInstancedIndirect(mesh, si, material, 0, m_cbArgs[si], 0, m_mpb);
            Graphics.ExecuteCommandBuffer(m_cmdMotionVector);
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
            m_positionOld = m_position;
            m_rotationOld = m_rotation;
            m_scaleOld = m_scale;

            var trans = GetComponent<Transform>();
            m_position = trans.position;
            m_rotation = trans.rotation;
            m_scale = trans.lossyScale;

            Flush();
        }

        void OnRenderObject()
        {
            FlushMotionVector();
        }

        void Start()
        {
            var trans = GetComponent<Transform>();
            m_position = m_positionOld = trans.position;
            m_rotation = m_rotationOld = trans.rotation;
            m_scale = m_scaleOld = trans.lossyScale;
        }
    }
}
