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

        EditorGUI.BeginChangeCheck();

        m_target.editMode = (NormalEditor.EditMode)EditorGUILayout.EnumPopup("Edit Mode", m_target.editMode);
        m_target.brushMode = (NormalEditor.BrushMode)EditorGUILayout.EnumPopup("Brush Mode", m_target.brushMode);
        m_target.selectMode = (NormalEditor.SelectMode)EditorGUILayout.EnumPopup("Select Mode", m_target.selectMode);
        m_target.mirroMode = (NormalEditor.MirrorMode)EditorGUILayout.EnumPopup("Mirror Mode", m_target.mirroMode);

        EditorGUILayout.Space();

        m_target.brushRadius = EditorGUILayout.FloatField("Brush Radius", m_target.brushRadius);
        m_target.brushPow = EditorGUILayout.FloatField("Brush Pow", m_target.brushPow);
        m_target.brushStrength = EditorGUILayout.FloatField("Brush Strength", m_target.brushStrength);

        EditorGUILayout.Space();

        m_target.showVertices = EditorGUILayout.Toggle("Show Vertices", m_target.showVertices);
        m_target.showNormals = EditorGUILayout.Toggle("Show Normals", m_target.showNormals);
        m_target.showTangents = EditorGUILayout.Toggle("Show Tangents", m_target.showTangents);
        m_target.showBinormals = EditorGUILayout.Toggle("Show Binormals", m_target.showBinormals);

        EditorGUILayout.Space();

        m_target.vertexColor = EditorGUILayout.ColorField("Vertex Color", m_target.vertexColor);
        m_target.vertexColor2 = EditorGUILayout.ColorField("Vertex Color (Selected)", m_target.vertexColor2);
        m_target.normalColor = EditorGUILayout.ColorField("Normal Color", m_target.normalColor);
        m_target.tangentColor = EditorGUILayout.ColorField("Tangent Color", m_target.tangentColor);
        m_target.binormalColor = EditorGUILayout.ColorField("Binormal Color", m_target.binormalColor);

        if (EditorGUI.EndChangeCheck())
        {
            SceneView.RepaintAll();
        }

        EditorGUILayout.Space();

        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Reset Normals"))
            m_target.ResetNormals();
        if (GUILayout.Button("Recalculate Tangents"))
            m_target.RecalculateTangents();
        GUILayout.EndHorizontal();

    }

    void OnUndoRedo()
    {
        if (!Application.isPlaying)
        {
            m_target.OnUndoRedo();
            SceneView.RepaintAll();
        }
    }

    void OnDisable()
    {
        Undo.undoRedoPerformed -= OnUndoRedo;
    }
}