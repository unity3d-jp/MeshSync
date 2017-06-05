using UnityEngine;
using UnityEditor;
using System.Collections;

[CustomEditor(typeof(NormalEditor))]
public class NormalEditorEditor : Editor
{
    NormalEditor m_target;

    int m_with = 1024;
    int m_height = 1024;


    void OnEnable()
    {
        m_target = target as NormalEditor;
    }

    void OnDisable()
    {
    }

    void OnSceneGUI()
    {
        m_target.OnSceneGUI();
    }

    public override void OnInspectorGUI()
    {
        //DrawDefaultInspector();

        EditorGUI.BeginChangeCheck();

        m_target.editMode = (NormalEditor.EditMode)EditorGUILayout.EnumPopup("Edit Mode", m_target.editMode);
        m_target.brushMode = (NormalEditor.BrushMode)EditorGUILayout.EnumPopup("Brush Mode", m_target.brushMode);
        m_target.selectMode = (NormalEditor.SelectMode)EditorGUILayout.EnumPopup("Select Mode", m_target.selectMode);
        m_target.mirroMode = (NormalEditor.MirrorMode)EditorGUILayout.EnumPopup("Mirror Mode", m_target.mirroMode);

        EditorGUILayout.Space();

        m_target.brushRadius = EditorGUILayout.Slider("Brush Radius", m_target.brushRadius, 0.01f, 1.0f);
        m_target.brushStrength = EditorGUILayout.Slider("Brush Strength", m_target.brushStrength, 0.01f, 1.0f);
        m_target.brushPow = EditorGUILayout.Slider("Brush Pow", m_target.brushPow, 0.01f, 1.0f);

        EditorGUILayout.Space();

        EditorGUILayout.LabelField("Visualize");
        m_target.showVertices = EditorGUILayout.Toggle("Vertices", m_target.showVertices);
        m_target.showNormals = EditorGUILayout.Toggle("Normals", m_target.showNormals);
        m_target.showTangents = EditorGUILayout.Toggle("Tangents", m_target.showTangents);
        m_target.showBinormals = EditorGUILayout.Toggle("Binormals", m_target.showBinormals);
        m_target.showTangentSpaceNormals = EditorGUILayout.Toggle("Tangent Space Normals", m_target.showTangentSpaceNormals);

        EditorGUILayout.Space();

        m_target.vertexColor = EditorGUILayout.ColorField("Vertex Color", m_target.vertexColor);
        m_target.vertexColor2 = EditorGUILayout.ColorField("Vertex Color (Selected)", m_target.vertexColor2);
        m_target.normalColor = EditorGUILayout.ColorField("Normal Color", m_target.normalColor);
        m_target.tangentColor = EditorGUILayout.ColorField("Tangent Color", m_target.tangentColor);
        m_target.binormalColor = EditorGUILayout.ColorField("Binormal Color", m_target.binormalColor);

        EditorGUILayout.Space();

        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Select All"))
        {
            if (m_target.SelectAll())
                m_target.UpdateSelection();
        }
        if (GUILayout.Button("Select None"))
        {
            if (m_target.SelectNone())
                m_target.UpdateSelection();
        }
        GUILayout.EndHorizontal();

        EditorGUILayout.Space();

        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Reset Normals"))
            m_target.ResetNormals();
        if (GUILayout.Button("Recalculate Tangents"))
            m_target.RecalculateTangents();
        GUILayout.EndHorizontal();

        EditorGUILayout.Space();

        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Export .obj file"))
        {
            string path = EditorUtility.SaveFilePanel("Export .obj file", "", m_target.name, "obj");
            if(path.Length > 0)
                ObjExporter.DoExport(m_target.gameObject, true, path);
        }
        GUILayout.EndHorizontal();

        EditorGUILayout.Space();

        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Bake to .png"))
        {
            string path = EditorUtility.SaveFilePanel("Export .png file", "", m_target.name + "_normal", "png");
            m_target.BakeToTexture(m_with, m_height, path);
        }
        if (GUILayout.Button("Bake to .exr"))
        {
            string path = EditorUtility.SaveFilePanel("Export .exr file", "", m_target.name + "_normal", "exr");
            m_target.BakeToTexture(m_with, m_height, path);
        }
        GUILayout.EndHorizontal();
        m_with = EditorGUILayout.IntField("Width", m_with);
        m_height = EditorGUILayout.IntField("Height", m_height);


        // BakeToTexture
        if (EditorGUI.EndChangeCheck())
            SceneView.RepaintAll();
    }
}