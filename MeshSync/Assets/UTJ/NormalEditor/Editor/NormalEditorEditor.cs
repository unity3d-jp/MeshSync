using UnityEngine;
using UnityEditor;
using System.Collections;

namespace UTJ.HumbleNormalEditor
{
    [CustomEditor(typeof(NormalEditor))]
    public class NormalEditorEditor : Editor
    {
        NormalEditor m_target;


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

            var settings = m_target.settings;

            EditorGUI.BeginChangeCheck();

            settings.editMode = (EditMode)EditorGUILayout.EnumPopup("Edit Mode", settings.editMode);
            settings.brushMode = (BrushMode)EditorGUILayout.EnumPopup("Brush Mode", settings.brushMode);
            settings.selectMode = (SelectMode)EditorGUILayout.EnumPopup("Select Mode", settings.selectMode);
            settings.mirrorMode = (MirrorMode)EditorGUILayout.EnumPopup("Mirror Mode", settings.mirrorMode);

            EditorGUILayout.Space();

            settings.selectFrontSideOnly = EditorGUILayout.Toggle("Select Front Side Only", settings.selectFrontSideOnly);
            settings.brushRadius = EditorGUILayout.Slider("Brush Radius", settings.brushRadius, 0.01f, 1.0f);
            settings.brushStrength = EditorGUILayout.Slider("Brush Strength", settings.brushStrength, 0.01f, 1.0f);
            settings.brushPow = EditorGUILayout.Slider("Brush Pow", settings.brushPow, 0.01f, 1.0f);

            EditorGUILayout.Space();


            settings.foldSelection = EditorGUILayout.Foldout(settings.foldSelection, "Selection");
            if (settings.foldSelection)
            {
                EditorGUI.indentLevel++;
                GUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Save", GUILayout.Width(70));
                for (int i = 0; i < 5; ++i)
                {
                    if (GUILayout.Button((i + 1).ToString()))
                        settings.selectionSets[i].selection = m_target.selection;
                }
                GUILayout.EndHorizontal();
                GUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Load", GUILayout.Width(70));
                for (int i = 0; i < 5; ++i)
                {
                    if (GUILayout.Button((i + 1).ToString()))
                        m_target.selection = settings.selectionSets[i].selection;
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            settings.foldCommands = EditorGUILayout.Foldout(settings.foldCommands, "Commands");
            if(settings.foldCommands)
            {
                EditorGUI.indentLevel++;

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * 16);
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
                GUILayout.Space(EditorGUI.indentLevel * 16);
                if (GUILayout.Button("Reset Normals"))
                    m_target.ResetNormals(false);
                if (GUILayout.Button("Reset Normals (Selected)"))
                    m_target.ResetNormals(true);
                GUILayout.EndHorizontal();

                EditorGUILayout.Space();

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * 16);
                if (GUILayout.Button("Recalculate Tangents"))
                    m_target.RecalculateTangents();
                GUILayout.EndHorizontal();

                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            settings.foldDisplay = EditorGUILayout.Foldout(settings.foldDisplay, "Display");
            if (settings.foldDisplay)
            {
                EditorGUI.indentLevel++;
                settings.showVertices = EditorGUILayout.Toggle("Vertices", settings.showVertices);
                settings.showNormals = EditorGUILayout.Toggle("Normals", settings.showNormals);
                settings.showTangents = EditorGUILayout.Toggle("Tangents", settings.showTangents);
                settings.showBinormals = EditorGUILayout.Toggle("Binormals", settings.showBinormals);
                settings.showTangentSpaceNormals = EditorGUILayout.Toggle("Tangent Space Normals", settings.showTangentSpaceNormals);

                settings.foldDisplayOptions = EditorGUILayout.Foldout(settings.foldDisplayOptions, "Display Options");
                if (settings.foldDisplayOptions)
                {
                    EditorGUI.indentLevel++;

                    settings.vertexSize = EditorGUILayout.Slider("Vertex Size", settings.vertexSize, 0.0f, 0.05f);
                    settings.normalSize = EditorGUILayout.Slider("Normal Size", settings.normalSize, 0.0f, 1.00f);
                    settings.tangentSize = EditorGUILayout.Slider("Tangent Size", settings.tangentSize, 0.0f, 1.00f);
                    settings.binormalSize = EditorGUILayout.Slider("Binormal Size", settings.binormalSize, 0.0f, 1.00f);

                    EditorGUILayout.Space();

                    settings.vertexColor = EditorGUILayout.ColorField("Vertex Color", settings.vertexColor);
                    settings.vertexColor2 = EditorGUILayout.ColorField("Vertex Color (Selected)", settings.vertexColor2);
                    settings.normalColor = EditorGUILayout.ColorField("Normal Color", settings.normalColor);
                    settings.tangentColor = EditorGUILayout.ColorField("Tangent Color", settings.tangentColor);
                    settings.binormalColor = EditorGUILayout.ColorField("Binormal Color", settings.binormalColor);

                    GUILayout.BeginHorizontal();
                    GUILayout.Space(EditorGUI.indentLevel * 16);
                    if (GUILayout.Button("Reset"))
                    {
                        settings.ResetDisplayOptions();
                    }
                    GUILayout.EndHorizontal();
                    EditorGUI.indentLevel--;
                }
                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            settings.foldExport = EditorGUILayout.Foldout(settings.foldExport, "Export");
            if (settings.foldExport)
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

            settings.foldBakeToTexture = EditorGUILayout.Foldout(settings.foldBakeToTexture, "Bake To Texture");
            if (settings.foldBakeToTexture)
            {
                EditorGUI.indentLevel++;

                settings.bakeFormat = (ImageFormat)EditorGUILayout.EnumPopup("Format", settings.bakeFormat);
                settings.bakeWidth = EditorGUILayout.IntField("Width", settings.bakeWidth);
                settings.bakeHeight = EditorGUILayout.IntField("Height", settings.bakeHeight);

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * 16);
                if (GUILayout.Button("Bake"))
                {
                    string path = settings.bakeFormat == ImageFormat.PNG ?
                        EditorUtility.SaveFilePanel("Export .png file", "", m_target.name + "_normal", "png") :
                        EditorUtility.SaveFilePanel("Export .exr file", "", m_target.name + "_normal", "exr");
                    m_target.BakeToTexture(settings.bakeWidth, settings.bakeHeight, path);
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            settings.foldBakeFromTexture = EditorGUILayout.Foldout(settings.foldBakeFromTexture, "Bake From Texture");
            if (settings.foldBakeFromTexture)
            {
                EditorGUI.indentLevel++;

                settings.bakeSource = EditorGUILayout.ObjectField("Source Texture", settings.bakeSource, typeof(Texture), true) as Texture;

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * 16);
                if (GUILayout.Button("Bake"))
                {
                    m_target.BakeFromTexture(settings.bakeSource);
                }
                GUILayout.EndHorizontal();

                EditorGUI.indentLevel--;
            }


            // BakeToTexture
            if (EditorGUI.EndChangeCheck())
                SceneView.RepaintAll();
        }
    }
}
