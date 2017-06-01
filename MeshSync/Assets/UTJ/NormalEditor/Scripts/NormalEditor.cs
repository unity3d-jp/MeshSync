using System.Collections;
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
    [SerializeField] Vector3[] m_normals;
    ComputeBuffer m_cbPoints;
    ComputeBuffer m_cbDirections;

    public Mesh mesh { get { return m_meshTarget; } }


    void SetupResources()
    {
        if (m_meshTarget == null)
        {
            m_meshTarget = GetComponent<MeshFilter>().mesh;
            m_normals = m_meshTarget.normals;

            if(m_cbPoints != null)
            {
                m_cbPoints.Release(); m_cbPoints = null;
                m_cbDirections.Release(); m_cbDirections = null;
            }
            m_cbPoints = new ComputeBuffer(m_normals.Length, 12);
            m_cbDirections = new ComputeBuffer(m_normals.Length, 12);
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
                32, 21, 20, 32, 22, 21,
            }, MeshTopology.Triangles, 0);
        }
        if (m_meshNormals == null)
        {
            m_meshNormals = new Mesh();
            m_meshNormals.vertices = new Vector3[2] { Vector3.zero, Vector3.zero };
            m_meshNormals.SetIndices(new int[2] { 0, 1 }, MeshTopology.Lines, 0);
        }
        if (m_matVertices == null)
        {
            m_matVertices = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/MeshSync/Shaders/VertexVisualizer.shader"));
        }
        if (m_matNormals == null)
        {
            m_matNormals = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/MeshSync/Shaders/NormalVisualizer.shader"));
        }
    }


    void OnEnable()
    {
        SetupResources();
    }

    public void ApplyNewNormals()
    {
        m_meshTarget.normals = m_normals;
    }
#endif
}