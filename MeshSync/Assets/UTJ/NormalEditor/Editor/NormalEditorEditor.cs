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