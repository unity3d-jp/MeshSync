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
        Undo.undoRedoPerformed += OnUndoRedo;
    }

    void OnSceneGUI()
    {
        m_target.OnSceneGUI();
    }

    public override void OnInspectorGUI()
    {
        DrawDefaultInspector();
    }

    void OnUndoRedo()
    {
        if (!Application.isPlaying)
        {
            m_target.OnUndoRedo();
        }
    }

    void OnDisable()
    {
        Undo.undoRedoPerformed -= OnUndoRedo;
    }
}