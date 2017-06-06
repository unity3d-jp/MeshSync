using UnityEngine;
using UnityEditor;
using System.Collections;

[CustomEditor(typeof(NormalEditor))]
public class NormalEditorEditor : Editor
{
    public enum ImageFormat
    {
        PNG,
        EXR,
    }

    NormalEditor m_target;
    bool m_foldDisplay = true;
    bool m_foldDisplayOptions;
    bool m_foldExport = true;
    bool m_foldBakeToTexture = true;
    bool m_foldBakeFromTexture = true;

    ImageFormat m_bakeFormat = ImageFormat.PNG;
    int m_bakeWith = 1024;
    int m_bakeHeight = 1024;

    Texture m_bakeSource;


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

        m_target.selectFrontSideOnly = EditorGUILayout.Toggle("Select Front Side Only", m_target.selectFrontSideOnly);
        m_target.brushRadius = EditorGUILayout.Slider("Brush Radius", m_target.brushRadius, 0.01f, 1.0f);
        m_target.brushStrength = EditorGUILayout.Slider("Brush Strength", m_target.brushStrength, 0.01f, 1.0f);
        m_target.brushPow = EditorGUILayout.Slider("Brush Pow", m_target.brushPow, 0.01f, 1.0f);

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
            m_target.ResetNormals(false);
        if (GUILayout.Button("Reset Normals (Selected)"))
            m_target.ResetNormals(true);
        GUILayout.EndHorizontal();
        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Recalculate Tangents"))
            m_target.RecalculateTangents();
        GUILayout.EndHorizontal();

        EditorGUILayout.Space();

        m_foldDisplay = EditorGUILayout.Foldout(m_foldDisplay, "Display");
        if(m_foldDisplay)
        {
            EditorGUI.indentLevel++;
            m_target.showVertices = EditorGUILayout.Toggle("Vertices", m_target.showVertices);
            m_target.showNormals = EditorGUILayout.Toggle("Normals", m_target.showNormals);
            m_target.showTangents = EditorGUILayout.Toggle("Tangents", m_target.showTangents);
            m_target.showBinormals = EditorGUILayout.Toggle("Binormals", m_target.showBinormals);
            m_target.showTangentSpaceNormals = EditorGUILayout.Toggle("Tangent Space Normals", m_target.showTangentSpaceNormals);

            m_foldDisplayOptions = EditorGUILayout.Foldout(m_foldDisplayOptions, "Display Options");
            if (m_foldDisplayOptions)
            {
                EditorGUI.indentLevel++;

                m_target.vertexSize = EditorGUILayout.Slider("Vertex Size", m_target.vertexSize, 0.0f, 0.05f);
                m_target.normalSize = EditorGUILayout.Slider("Normal Size", m_target.normalSize, 0.0f, 1.00f);
                m_target.tangentSize = EditorGUILayout.Slider("Tangent Size", m_target.tangentSize, 0.0f, 1.00f);
                m_target.binormalSize = EditorGUILayout.Slider("Binormal Size", m_target.binormalSize, 0.0f, 1.00f);

                EditorGUILayout.Space();

                m_target.vertexColor = EditorGUILayout.ColorField("Vertex Color", m_target.vertexColor);
                m_target.vertexColor2 = EditorGUILayout.ColorField("Vertex Color (Selected)", m_target.vertexColor2);
                m_target.normalColor = EditorGUILayout.ColorField("Normal Color", m_target.normalColor);
                m_target.tangentColor = EditorGUILayout.ColorField("Tangent Color", m_target.tangentColor);
                m_target.binormalColor = EditorGUILayout.ColorField("Binormal Color", m_target.binormalColor);

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * 16);
                if (GUILayout.Button("Reset"))
                {
                    m_target.ResetDisplayOptions();
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
            }
            EditorGUI.indentLevel--;
        }

        EditorGUILayout.Space();

        m_foldExport = EditorGUILayout.Foldout(m_foldExport, "Export");
        if(m_foldExport)
        {
            EditorGUI.indentLevel++;

            GUILayout.BeginHorizontal();
            GUILayout.Space(EditorGUI.indentLevel * 16);
            if (GUILayout.Button("Export .obj file"))
            {
                string path = EditorUtility.SaveFilePanel("Export .obj file", "", m_target.name, "obj");
                if (path.Length > 0)
                    ObjExporter.DoExport(m_target.gameObject, true, path);
            }
            GUILayout.EndHorizontal();

            EditorGUI.indentLevel--;
        }


        EditorGUILayout.Space();

        m_foldBakeToTexture = EditorGUILayout.Foldout(m_foldBakeToTexture, "Bake To Texture");
        if (m_foldBakeToTexture)
        {
            EditorGUI.indentLevel++;

            m_bakeFormat = (ImageFormat)EditorGUILayout.EnumPopup("Format", m_bakeFormat);
            m_bakeWith = EditorGUILayout.IntField("Width", m_bakeWith);
            m_bakeHeight = EditorGUILayout.IntField("Height", m_bakeHeight);

            GUILayout.BeginHorizontal();
            GUILayout.Space(EditorGUI.indentLevel * 16);
            if (GUILayout.Button("Bake"))
            {
                string path = m_bakeFormat == ImageFormat.PNG ?
                    EditorUtility.SaveFilePanel("Export .png file", "", m_target.name + "_normal", "png") :
                    EditorUtility.SaveFilePanel("Export .exr file", "", m_target.name + "_normal", "exr");
                m_target.BakeToTexture(m_bakeWith, m_bakeHeight, path);
            }
            GUILayout.EndHorizontal();
            EditorGUI.indentLevel--;
        }

        EditorGUILayout.Space();

        m_foldBakeFromTexture = EditorGUILayout.Foldout(m_foldBakeFromTexture, "Bake From Texture");
        if (m_foldBakeFromTexture)
        {
            EditorGUI.indentLevel++;

            m_bakeSource = EditorGUILayout.ObjectField("Source Texture", m_bakeSource, typeof(Texture), true) as Texture;

            GUILayout.BeginHorizontal();
            GUILayout.Space(EditorGUI.indentLevel * 16);
            if (GUILayout.Button("Bake"))
            {
                m_target.BakeFromTexture(m_bakeSource);
            }
            GUILayout.EndHorizontal();

            EditorGUI.indentLevel--;
        }


        // BakeToTexture
        if (EditorGUI.EndChangeCheck())
        SceneView.RepaintAll();
    }
}