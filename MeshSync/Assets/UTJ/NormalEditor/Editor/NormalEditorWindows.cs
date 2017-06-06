using UnityEngine;
using UnityEditor;

namespace UTJ.HumbleNormalEditor
{
    public class NormalEditorWindow : EditorWindow
    {
        public static bool isOpen;
        Vector2 m_scrollPos;
        NormalEditor m_target;
        MeshRenderer m_mr;

        [MenuItem("Window/Normal Editor")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<NormalEditorWindow>();
            window.titleContent = new GUIContent("Normal Editor");
            window.Show();
            window.OnSelectionChange();
        }

        public static void DrawNormalEditor(NormalEditor editor)
        {
            var settings = editor.settings;
            int indentSize = 18;

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
                        settings.selectionSets[i].selection = editor.selection;
                }
                GUILayout.EndHorizontal();

                EditorGUILayout.Space();

                GUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Load", GUILayout.Width(70));
                for (int i = 0; i < 5; ++i)
                {
                    if (GUILayout.Button((i + 1).ToString()))
                        editor.selection = settings.selectionSets[i].selection;
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            settings.foldCommands = EditorGUILayout.Foldout(settings.foldCommands, "Commands");
            if (settings.foldCommands)
            {
                EditorGUI.indentLevel++;

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Select All"))
                {
                    if (editor.SelectAll())
                        editor.UpdateSelection();
                }
                if (GUILayout.Button("Clear Selection"))
                {
                    if (editor.SelectNone())
                        editor.UpdateSelection();
                }
                GUILayout.EndHorizontal();

                EditorGUILayout.Space();

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Reset Normals"))
                    editor.ResetNormals(false);
                if (GUILayout.Button("Reset Normals (Selection)"))
                    editor.ResetNormals(true);
                GUILayout.EndHorizontal();

                EditorGUILayout.Space();

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Recalculate Tangents"))
                    editor.RecalculateTangents();
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
                    GUILayout.Space(EditorGUI.indentLevel * indentSize);
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

            settings.foldBakeToVertexColor = EditorGUILayout.Foldout(settings.foldBakeToVertexColor, "Vertex Color Conversion");
            if (settings.foldBakeToVertexColor)
            {
                EditorGUI.indentLevel++;

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Convert To Vertex Color"))
                    editor.BakeToVertexColor();
                if (GUILayout.Button("Convert From Vertex Color"))
                    editor.LoadVertexColor();
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
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Bake"))
                {
                    string path = settings.bakeFormat == ImageFormat.PNG ?
                        EditorUtility.SaveFilePanel("Export .png file", "", editor.name + "_normal", "png") :
                        EditorUtility.SaveFilePanel("Export .exr file", "", editor.name + "_normal", "exr");
                    editor.BakeToTexture(settings.bakeWidth, settings.bakeHeight, path);
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            settings.foldLoadTexture = EditorGUILayout.Foldout(settings.foldLoadTexture, "Load Texture");
            if (settings.foldLoadTexture)
            {
                EditorGUI.indentLevel++;

                settings.bakeSource = EditorGUILayout.ObjectField("Source Texture", settings.bakeSource, typeof(Texture), true) as Texture;

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Load"))
                    editor.LoadTexture(settings.bakeSource);
                GUILayout.EndHorizontal();

                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();

            settings.foldExport = EditorGUILayout.Foldout(settings.foldExport, "Export");
            if (settings.foldExport)
            {
                EditorGUI.indentLevel++;

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Export .obj file"))
                {
                    string path = EditorUtility.SaveFilePanel("Export .obj file", "", editor.name, "obj");
                    if (path.Length > 0)
                        ObjExporter.Export(editor.gameObject, path,
                            new ObjExporter.Settings { includeChildren=false, applyTransform=false,});
                }
                GUILayout.EndHorizontal();

                EditorGUI.indentLevel--;
            }

            EditorGUILayout.Space();


            if (EditorGUI.EndChangeCheck())
                SceneView.RepaintAll();
        }


        private void OnEnable()
        {
            isOpen = true;
        }

        private void OnDisable()
        {
            isOpen = false;
        }

        private void OnGUI()
        {
            if (m_target != null)
            {
                m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);
                DrawNormalEditor(m_target);
                EditorGUILayout.EndScrollView();
            }
            else if(m_mr != null)
            {
                if (GUILayout.Button("Add Normal Editor"))
                {
                    m_mr.gameObject.AddComponent<NormalEditor>();
                    OnSelectionChange();
                }
            }
        }

        private void OnSelectionChange()
        {
            m_target = null;
            m_mr = null;
            if (Selection.activeGameObject != null)
            {
                m_target = Selection.activeGameObject.GetComponent<NormalEditor>();
                m_mr = Selection.activeGameObject.GetComponent<MeshRenderer>();
            }
            Repaint();
        }
    }
}
