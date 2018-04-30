using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

public class VisualizeSkeleton : MonoBehaviour
{
#if UNITY_EDITOR
    public Material m_material;
    Mesh m_mesh;
    List<Vector3> m_vertices;
    List<int> m_indices;


    [MenuItem("Debug/Toggle Visualize Skeleton")]
    public static void DumpTransform()
    {
        var go = Selection.activeGameObject;
        if (go != null)
        {
            var smr = go.GetComponent<SkinnedMeshRenderer>();
            if (smr != null && smr.rootBone != null)
            {
                var vs = smr.rootBone.GetComponent<VisualizeSkeleton>();
                if (vs == null)
                    smr.rootBone.gameObject.AddComponent<VisualizeSkeleton>();
                else
                    DestroyImmediate(vs);
            }
        }
    }

    void OnDrawGizmos()
    {
        if (m_vertices == null)
            m_vertices = new List<Vector3>();
        if (m_indices == null)
            m_indices = new List<int>();
        if (m_mesh == null)
        {
            m_mesh = new Mesh();
            m_mesh.MarkDynamic();
        }
        if(m_material == null)
        {
            m_material = new Material(Shader.Find("MeshSync/Fill"));
        }

        BuildLinesRecursive(GetComponent<Transform>(), true);
        m_mesh.Clear();
        m_mesh.SetVertices(m_vertices);
        m_mesh.SetIndices(m_indices.ToArray(), MeshTopology.Lines, 0);

        if (m_material != null)
            m_material.SetPass(0);
        Graphics.DrawMeshNow(m_mesh, Matrix4x4.identity);
        m_vertices.Clear();
        m_indices.Clear();
    }

    void BuildLinesRecursive(Transform node, bool isRoot)
    {
        foreach(var child in node.GetComponentsInChildren<Transform>())
        {
            if (child == node)
                continue;

            if (!isRoot)
            {
                m_vertices.Add(child.parent.position);
                m_vertices.Add(child.position);

                m_indices.Add(m_indices.Count);
                m_indices.Add(m_indices.Count);
            }

            BuildLinesRecursive(child, false);
        }
    }
#endif
}
