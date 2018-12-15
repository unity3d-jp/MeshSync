using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

public class VisualizeSkeleton : MonoBehaviour
{
#if UNITY_EDITOR
    public Material m_material;
    public float m_nodeSize = 0.01f;
    Mesh m_meshLines;
    Mesh m_meshCube;
    List<Vector3> m_vertices;
    List<int> m_indices;
    Transform m_root;
    Transform[] m_bones;


    [MenuItem("Debug/Visualize Skeleton: On")]
    public static void On()
    {
        EachSkinnedMeshRenderer(smr => On(smr));
    }

    [MenuItem("Debug/Visualize Skeleton: Off")]
    public static void Off()
    {
        EachSkinnedMeshRenderer(smr => Off(smr));
    }

    static void EachSkinnedMeshRenderer(System.Action<SkinnedMeshRenderer> act)
    {
        var selection = Selection.gameObjects;
        if (selection.Length > 0)
        {
            foreach (var go in selection)
            {
                var smr = go.GetComponent<SkinnedMeshRenderer>();
                if (smr != null)
                    act.Invoke(smr);
            }
        }
        else
        {
            foreach (var smr in GameObject.FindObjectsOfType<SkinnedMeshRenderer>())
                act.Invoke(smr);
        }
    }

    static void On(SkinnedMeshRenderer smr)
    {
        if (smr != null && smr.rootBone != null)
        {
            var vs = smr.rootBone.GetComponent<VisualizeSkeleton>();
            if (vs == null)
            {
                vs = smr.rootBone.gameObject.AddComponent<VisualizeSkeleton>();
                vs.root = smr.rootBone;
                vs.bones = smr.bones;
            }
        }
    }

    static void Off(SkinnedMeshRenderer smr)
    {
        if (smr != null && smr.rootBone != null)
        {
            var vs = smr.rootBone.GetComponent<VisualizeSkeleton>();
            if (vs != null)
                DestroyImmediate(vs);
        }
    }

    static Mesh BuildCubeMesh(float size)
    {
        float l = size;
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

        var mesh = new Mesh();
        mesh.vertices = new Vector3[] {
            p[0], p[1], p[2], p[3],
            p[7], p[4], p[0], p[3],
            p[4], p[5], p[1], p[0],
            p[6], p[7], p[3], p[2],
            p[5], p[6], p[2], p[1],
            p[7], p[6], p[5], p[4],
        };
        mesh.SetIndices(new int[] {
            3, 1, 0, 3, 2, 1,
            7, 5, 4, 7, 6, 5,
            11, 9, 8, 11, 10, 9,
            15, 13, 12, 15, 14, 13,
            19, 17, 16, 19, 18, 17,
            23, 21, 20, 23, 22, 21,
        }, MeshTopology.Triangles, 0);
        mesh.UploadMeshData(false);
        return mesh;
    }

    public Transform root { set { m_root = value; } }
    public Transform[] bones { set { m_bones = value; } }

    void OnDrawGizmos()
    {
        // initialize resources
        if (m_vertices == null)
            m_vertices = new List<Vector3>();
        if (m_indices == null)
            m_indices = new List<int>();
        if (m_meshLines == null)
        {
            m_meshLines = new Mesh();
            m_meshLines.MarkDynamic();
        }
        if (m_meshCube == null)
            m_meshCube = BuildCubeMesh(0.5f);
        if (m_material == null)
            m_material = new Material(Shader.Find("MeshSync/Fill"));

        // build mesh
        m_vertices.Clear();
        m_indices.Clear();
        foreach (var bone in m_bones)
        {
            if (bone == null || bone == m_root)
                continue;

            m_vertices.Add(bone.parent.position);
            m_vertices.Add(bone.position);

            m_indices.Add(m_indices.Count);
            m_indices.Add(m_indices.Count);
        }
        m_meshLines.Clear();
        m_meshLines.SetVertices(m_vertices);
        m_meshLines.SetIndices(m_indices.ToArray(), MeshTopology.Lines, 0);

        // draw
        if (m_material != null)
            m_material.SetPass(0);
        Graphics.DrawMeshNow(m_meshLines, Matrix4x4.identity);

        var scale = new Vector3(m_nodeSize, m_nodeSize, m_nodeSize);
        foreach (var bone in m_bones)
        {
            if (bone == null)
                continue;
            Graphics.DrawMeshNow(m_meshCube, Matrix4x4.TRS(bone.position, Quaternion.identity, scale));
        }
    }
#endif
}
