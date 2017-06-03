using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

[ExecuteInEditMode]
[RequireComponent(typeof(MeshRenderer))]
public class NormalEditor : MonoBehaviour
{
#if UNITY_EDITOR
    [SerializeField] Mesh m_meshTarget;
    [SerializeField] Mesh m_meshVertices;
    [SerializeField] Mesh m_meshNormals;
    [SerializeField] Material m_matVertices;
    [SerializeField] Material m_matNormals;
    [SerializeField] List<Vector3> m_points = new List<Vector3>();
    [SerializeField] List<Vector3> m_normals = new List<Vector3>();
    [SerializeField] List<Vector4> m_tangents = new List<Vector4>();
    ComputeBuffer m_cbPoints;
    ComputeBuffer m_cbNormals;
    ComputeBuffer m_cbTangents;

    public Mesh mesh { get { return m_meshTarget; } }


    void SetupResources()
    {
        if (m_meshTarget == null ||
            m_meshTarget.vertexCount != GetComponent<MeshFilter>().sharedMesh.vertexCount)
        {
            m_meshTarget = GetComponent<MeshFilter>().sharedMesh;
            m_points = new List<Vector3>(m_meshTarget.vertices);
            m_normals = new List<Vector3>(m_meshTarget.normals);
            m_tangents = new List<Vector4>(m_meshTarget.tangents);

            ReleaseComputeBuffers();
            if (m_points.Count > 0) m_cbPoints = new ComputeBuffer(m_points.Count, 12);
            if (m_normals.Count > 0) m_cbNormals = new ComputeBuffer(m_normals.Count, 12);
            if (m_tangents.Count > 0) m_cbTangents = new ComputeBuffer(m_tangents.Count, 16);
        }

        if (m_meshVertices == null)
        {
            float l = 0.5f;
            var p = new Vector3[] {
                new Vector3(-l,-l, l),
                new Vector3( l,-l, l),
                new Vector3( l,-l,-l),
                new Vector3(-l,-l,-l),

                new Vector3(-l, l, l),
                new Vector3( l, l, l),
                new Vector3( l, l,-l),
                new Vector3(-l, l,-l),
            };

            m_meshVertices = new Mesh();
            m_meshVertices.vertices = new Vector3[] {
                p[0], p[1], p[2], p[3],
                p[7], p[4], p[0], p[3],
                p[4], p[5], p[1], p[0],
                p[6], p[7], p[3], p[2],
                p[5], p[6], p[2], p[1],
                p[7], p[6], p[5], p[4],
            };
            m_meshVertices.SetIndices(new int[] {
                3, 1, 0, 3, 2, 1,
                7, 5, 4, 7, 6, 5,
                11, 9, 8, 11, 10, 9,
                15, 13, 12, 15, 14, 13,
                19, 17, 16, 19, 18, 17,
                23, 21, 20, 23, 22, 21,
            }, MeshTopology.Triangles, 0);
        }

        if (m_meshNormals == null)
        {
            m_meshNormals = new Mesh();
            m_meshNormals.vertices = new Vector3[2] { Vector3.zero, Vector3.zero };
            m_meshNormals.uv = new Vector2[2] { Vector2.zero, Vector2.one };
            m_meshNormals.SetIndices(new int[2] { 0, 1 }, MeshTopology.Lines, 0);
        }

        if (m_matVertices == null)
        {
            m_matVertices = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/NormalEditor/Shaders/VertexVisualizer.shader"));
        }
        if (m_matNormals == null)
        {
            m_matNormals = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/NormalEditor/Shaders/NormalVisualizer.shader"));
        }
    }

    void ReleaseComputeBuffers()
    {
        if (m_cbPoints != null) { m_cbPoints.Release(); m_cbPoints = null; }
        if (m_cbNormals != null) { m_cbNormals.Release(); m_cbNormals = null; }
        if (m_cbTangents != null) { m_cbTangents.Release(); m_cbTangents = null; }
    }


    void OnEnable()
    {
        SetupResources();
    }

    void OnDestroy()
    {
        ReleaseComputeBuffers();
    }

    void Reset()
    {
        SetupResources();
    }

    public void ApplyNewNormals()
    {
        m_meshTarget.SetNormals(m_normals);
    }
#endif
}