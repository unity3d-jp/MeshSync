using UnityEngine;
using UnityEditor;

namespace UTJ.HumbleNormalEditor
{
    public class NormalEditorCommandWindow : EditorWindow
    {
        public static bool isOpen;
        Vector2 m_scrollPos;
        NormalEditor m_target;
        Transform m_pivot;

        bool foldEdit = true;
        bool foldCommands = true;
        bool foldInExport = true;
        bool foldDisplay = true;

        int editIndex;
        int commandIndex;
        int inexportIndex;

        public Vector3 setValue = Vector3.up;
        public Vector3 moveAmount;
        public bool rotateUsePivot;
        public Vector3 rotateAmount;
        public Vector3 scaleAmount;
        public float equalizeRadius = 0.5f;
        public float equalizeAmount = 1.0f;
        public GameObject projector;


        [MenuItem("Window/Normal Editor Commands")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<NormalEditorCommandWindow>();
            window.titleContent = new GUIContent("Normal Commands");
            window.OnSelectionChange();
            window.Show();
        }

        void RepaintAllViews()
        {
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }


        static readonly string[] strEdit = new string[] {
            "Paint",
            "Selection",
        };

        void DrawEditPanel()
        {
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(18));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(100));
            editIndex = GUILayout.SelectionGrid(editIndex, strEdit, 1);
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(10));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();
            // 
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();
        }


        static readonly string[] strCommands = new string[] {
            "Set",
            "Move",
            "Rotate",
            "Scale",
            "Equalize",
            "Projection",
            "Reset",
        };

        void DrawCommandPanel()
        {
            var settings = m_target.settings;

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(18));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(100));
            commandIndex = GUILayout.SelectionGrid(commandIndex, strCommands, 1);
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(10));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();

            if (commandIndex == 0)
            {
                setValue = EditorGUILayout.Vector3Field("Value", setValue);
                if (GUILayout.Button("Set"))
                {
                    m_target.ApplySet(setValue);
                }
            }
            else if (commandIndex == 1)
            {
                moveAmount = EditorGUILayout.Vector3Field("Move Amount", moveAmount);
                if (GUILayout.Button("Move"))
                {
                    m_target.ApplyMove(moveAmount);
                }
            }
            else if (commandIndex == 2)
            {
                rotateAmount = EditorGUILayout.Vector3Field("Rotate Amount", rotateAmount);
                if (GUILayout.Button("Rotate"))
                {
                    m_target.ApplyRotate(Quaternion.Euler(rotateAmount.x, rotateAmount.y, rotateAmount.z));
                }
            }
            else if (commandIndex == 3)
            {
                scaleAmount = EditorGUILayout.Vector3Field("Scale Amount", scaleAmount);
                if (GUILayout.Button("Scale") && m_pivot != null)
                {
                    m_target.ApplyScale(scaleAmount, m_pivot.position);
                }
            }
            else if (commandIndex == 4)
            {
                equalizeRadius = EditorGUILayout.FloatField("Equalize Radius", equalizeRadius);
                equalizeAmount = EditorGUILayout.FloatField("Equalize Amount", equalizeAmount);
                if (GUILayout.Button("Equalize"))
                {
                    m_target.ApplyEqualize(equalizeRadius, equalizeAmount);
                }
            }
            else if (commandIndex == 5)
            {
                projector = EditorGUILayout.ObjectField("Projector", projector, typeof(GameObject), true) as GameObject;
                if (GUILayout.Button("Project"))
                {
                    m_target.ApplyProjection(projector);
                }
            }
            else if (commandIndex == 6)
            {
                EditorGUILayout.BeginHorizontal();
                if (GUILayout.Button("Reset (Selection)"))
                {
                    m_target.ResetNormals(true);
                }
                else if (GUILayout.Button("Reset (All)"))
                {
                    m_target.ResetNormals(false);
                }
                EditorGUILayout.EndHorizontal();
            }
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();
        }


        static readonly string[] strInExport = new string[] {
            "Vertex Color",
            "Bake To Texture",
            "Load Texture",
            "Export .obj",
        };

        void DrawInExportPanel()
        {
            var settings = m_target.settings;

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(18));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(100));
            inexportIndex = GUILayout.SelectionGrid(inexportIndex, strInExport, 1);
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(10));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();

            if(inexportIndex == 0)
            {
                GUILayout.BeginHorizontal();
                if (GUILayout.Button("Convert To Vertex Color"))
                    m_target.BakeToVertexColor();
                if (GUILayout.Button("Convert From Vertex Color"))
                    m_target.LoadVertexColor();
                GUILayout.EndHorizontal();
            }
            else if (inexportIndex == 1)
            {
                settings.bakeFormat = (ImageFormat)EditorGUILayout.EnumPopup("Format", settings.bakeFormat);
                settings.bakeWidth = EditorGUILayout.IntField("Width", settings.bakeWidth);
                settings.bakeHeight = EditorGUILayout.IntField("Height", settings.bakeHeight);

                if (GUILayout.Button("Bake"))
                {
                    string path = settings.bakeFormat == ImageFormat.PNG ?
                        EditorUtility.SaveFilePanel("Export .png file", "", m_target.name + "_normal", "png") :
                        EditorUtility.SaveFilePanel("Export .exr file", "", m_target.name + "_normal", "exr");
                    m_target.BakeToTexture(settings.bakeWidth, settings.bakeHeight, path);
                }
            }
            else if (inexportIndex == 2)
            {
                settings.bakeSource = EditorGUILayout.ObjectField("Source Texture", settings.bakeSource, typeof(Texture), true) as Texture;

                if (GUILayout.Button("Load"))
                    m_target.LoadTexture(settings.bakeSource);
            }
            else if (inexportIndex == 3)
            {
                settings.objFlipHandedness = EditorGUILayout.Toggle("Flip Handedness", settings.objFlipHandedness);
                settings.objFlipFaces = EditorGUILayout.Toggle("Flip Faces", settings.objFlipFaces);
                settings.objApplyTransform = EditorGUILayout.Toggle("Apply Transform", settings.objApplyTransform);
                settings.objMakeSubmeshes = EditorGUILayout.Toggle("Make Submeshes", settings.objMakeSubmeshes);
                settings.objIncludeChildren = EditorGUILayout.Toggle("Include Children", settings.objIncludeChildren);

                if (GUILayout.Button("Export .obj file"))
                {
                    string path = EditorUtility.SaveFilePanel("Export .obj file", "", m_target.name, "obj");
                    if (path.Length > 0)
                        ObjExporter.Export(m_target.gameObject, path, new ObjExporter.Settings {
                            flipFaces = settings.objFlipFaces,
                            flipHandedness = settings.objFlipHandedness,
                            includeChildren = settings.objIncludeChildren,
                            makeSubmeshes = settings.objMakeSubmeshes,
                            applyTransform = settings.objApplyTransform,
                        });
                }

            }
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();
        }


        void DrawDisplayPanel()
        {
            var settings = m_target.settings;

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(18));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(100));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(10));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();
            settings.showVertices = EditorGUILayout.Toggle("Vertices", settings.showVertices);
            settings.showNormals = EditorGUILayout.Toggle("Normals", settings.showNormals);
            settings.showTangents = EditorGUILayout.Toggle("Tangents", settings.showTangents);
            settings.showBinormals = EditorGUILayout.Toggle("Binormals", settings.showBinormals);
            EditorGUI.indentLevel++;
            settings.showSelectedOnly = EditorGUILayout.Toggle("Only Selected", settings.showSelectedOnly);
            EditorGUI.indentLevel--;

            settings.modelOverlay = (ModelOverlay)EditorGUILayout.EnumPopup("Overlay", settings.modelOverlay);


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
                GUILayout.Space(EditorGUI.indentLevel * 18);
                if (GUILayout.Button("Reset"))
                {
                    settings.ResetDisplayOptions();
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
            }
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();
        }


        private void OnGUI()
        {
            if (m_target == null)
                return;

            EditorGUI.BeginChangeCheck();
            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);

            foldEdit = EditorGUILayout.Foldout(foldEdit, "Edit");
            if (foldEdit)
                DrawEditPanel();

            EditorGUILayout.Space();

            foldCommands = EditorGUILayout.Foldout(foldCommands, "Commands");
            if(foldCommands)
                DrawCommandPanel();

            EditorGUILayout.Space();

            foldDisplay = EditorGUILayout.Foldout(foldDisplay, "Display");
            if (foldDisplay)
                DrawDisplayPanel();

            EditorGUILayout.Space();

            foldInExport = EditorGUILayout.Foldout(foldInExport, "Import / Export");
            if (foldInExport)
                DrawInExportPanel();

            EditorGUILayout.EndScrollView();
            if (EditorGUI.EndChangeCheck())
                RepaintAllViews();
        }

        private void OnEnable()
        {
            isOpen = true;
        }

        private void OnDisable()
        {
            isOpen = false;
        }

        private void OnSelectionChange()
        {
            m_target = null;
            if (Selection.activeGameObject != null)
            {
                m_target = Selection.activeGameObject.GetComponent<NormalEditor>();
            }
            Repaint();
        }
    }
}
