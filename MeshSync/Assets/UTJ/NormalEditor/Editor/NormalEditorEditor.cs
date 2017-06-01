using UnityEngine;
using UnityEditor;
using System.Collections;

[CustomEditor(typeof(NormalEditor))]
public class NormalEditorEditor : Editor
{
    NormalEditor m_target;
    void OnEnable()
    {
        m_target = target as NormalEditor;
        Undo.undoRedoPerformed += ApplyNewNormals;
    }

    void OnSceneGUI()
    {
        if (m_target == null || m_target.mesh == null) return;

        for (int i = 0; i < m_target.mesh.vertexCount; i++)
        {
            //Handles.color = Color.blue;
            //Handles.matrix = m_norm.transform.localToWorldMatrix;
            //Handles.Label(m_norm.Mesh.vertices[i], i.ToString());

            Handles.color = Color.yellow;
            Handles.DrawLine(
                m_target.mesh.vertices[i],
                m_target.mesh.vertices[i] + m_target.mesh.normals[i] * 0.1f);
        }
    }

    public override void OnInspectorGUI()
    {
        EditorGUI.BeginChangeCheck();
        DrawDefaultInspector();
        if (EditorGUI.EndChangeCheck())
        {
            ApplyNewNormals();
        }
    }

    void ApplyNewNormals()
    {
        if (!Application.isPlaying)
        {
            m_target.ApplyNewNormals();
        }
    }

    void OnDisable()
    {
        Undo.undoRedoPerformed -= ApplyNewNormals;
    }
}