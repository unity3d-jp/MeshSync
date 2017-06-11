using UnityEngine;
using UnityEditor;

namespace UTJ.NormalPainter
{
    public class NormalPainterWindow : EditorWindow
    {
        public static bool isOpen;

        Vector2 m_scrollPos;
        NormalPainter m_target;
        GameObject m_active;



        string tips = "";



        [MenuItem("Window/Normal Painter")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<NormalPainterWindow>();
            window.titleContent = new GUIContent("Normal Painter");
            window.Show();
            window.OnSelectionChange();
        }



        private void OnEnable()
        {
            isOpen = true;
            SceneView.onSceneGUIDelegate += OnSceneGUI;
        }

        private void OnDisable()
        {
            SceneView.onSceneGUIDelegate -= OnSceneGUI;
            isOpen = false;
        }

        private void OnSceneGUI(SceneView sceneView)
        {
            if (m_target != null && m_target.settings.editing)
            {
                Tools.current = Tool.None;

                if (HandleShortcutKeys())
                {
                    Event.current.Use();
                    RepaintAllViews();
                }
                else
                {
                    int ret = m_target.OnSceneGUI();
                    if ((ret & (int)SceneGUIState.Repaint) != 0)
                        RepaintAllViews();
                }
            }
        }

        private void OnGUI()
        {
            if (m_target != null)
            {
                if (!m_target.isActiveAndEnabled)
                {
                    EditorGUILayout.LabelField("(Enable " + m_target.name + " to show Normal Painter)");
                }
                else
                {
                    var tooltipHeight = 0;
                    var windowHeight = position.height;
                    var settings = m_target.settings;

                    EditorGUILayout.BeginHorizontal();
                    EditorGUI.BeginChangeCheck();
                    settings.editing = GUILayout.Toggle(settings.editing, EditorGUIUtility.IconContent("EditCollider"),
                        "Button", GUILayout.Width(33), GUILayout.Height(23));
                    if (EditorGUI.EndChangeCheck())
                    {
                        if (settings.editing)
                        {
                            Tools.current = Tool.None;
                        }
                        else
                        {
                            Tools.current = Tool.Move;
                        }
                    }
                    GUILayout.Label("Edit Normals");
                    EditorGUILayout.EndHorizontal();


                    if (settings.editing)
                    {
                        EditorGUILayout.BeginVertical(GUILayout.Height(windowHeight - tooltipHeight));
                        m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);
                        DrawNormalPainter();
                        EditorGUILayout.EndScrollView();

                        EditorGUILayout.LabelField(tips);
                        EditorGUILayout.EndVertical();
                    }
                }
            }
            else if (m_active != null)
            {
                if (GUILayout.Button("Add Normal Painter to " + m_active.name))
                {
                    m_active.AddComponent<NormalPainter>();
                    OnSelectionChange();
                }
            }
        }

        private void OnSelectionChange()
        {
            m_target = null;
            m_active = null;
            if (Selection.activeGameObject != null)
            {
                m_target = Selection.activeGameObject.GetComponent<NormalPainter>();
                if(m_target == null)
                {
                    var activeGameObject = Selection.activeGameObject;
                    if ( Selection.activeGameObject.GetComponent<MeshRenderer>() != null ||
                         Selection.activeGameObject.GetComponent<SkinnedMeshRenderer>() != null)
                    {
                        m_active = activeGameObject;
                    }
                }
            }
            Repaint();
        }






        void RepaintAllViews()
        {
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }

        static readonly int indentSize = 18;
        static readonly int spaceSize = 5;
        static readonly int c1Width = 100;

        static readonly string[] strBrushTypes = new string[] {
            "Paint",
            "Pinch",
            "Equalize",
            "Reset",
        };
        static readonly string[] strSelectMode = new string[] {
            "Single",
            "Rect",
            "Lasso",
            "Brush",
        };

        static readonly string[] strCommands = new string[] {
            "Selection",
            "Brush",
            "Assign",
            "Move",
            "Rotate",
            "Scale",
            "Equalize",
            "Projection",
            "Reset",
        };

        void DrawEditPanel()
        {
            var settings = m_target.settings;

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(indentSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(c1Width));
            {
                var prev = settings.editMode;
                settings.editMode = (EditMode)GUILayout.SelectionGrid((int)settings.editMode, strCommands, 1);
                if (settings.editMode != prev)
                {
                    switch (settings.editMode)
                    {
                        case EditMode.Select:
                        case EditMode.Assign:
                        case EditMode.Move:
                        case EditMode.Rotate:
                        case EditMode.Scale:
                        case EditMode.Equalize:
                        case EditMode.Projection:
                        case EditMode.Reset:
                            tips = "Shift+LB: Add selection, Ctrl+LB: Subtract selection";
                            break;
                        case EditMode.Brush:
                            tips = "Shift+Wheel: Change radius, Ctrl+Wheel: Change strength, Alt+Wheel: Change falloff";
                            break;
                    }
                }
            }
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(spaceSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();

            if (settings.editMode == EditMode.Select)
            {
                settings.selectMode = (SelectMode)GUILayout.SelectionGrid((int)settings.selectMode, strSelectMode, 4);
                EditorGUILayout.Space();
                if (settings.selectMode == SelectMode.Brush)
                {
                    settings.brushRadius = EditorGUILayout.Slider("Brush Radius", settings.brushRadius, 0.01f, 1.0f);
                    settings.brushStrength = EditorGUILayout.Slider("Brush Strength", settings.brushStrength, -1.0f, 1.0f);
                    settings.brushFalloff = EditorGUILayout.Slider("Brush Falloff", settings.brushFalloff, 0.0f, 2.0f);
                }
                else
                {
                    settings.selectFrontSideOnly = EditorGUILayout.Toggle("Front Side Only", settings.selectFrontSideOnly);
                }
                EditorGUILayout.Space();

                GUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Save", GUILayout.Width(50));
                for (int i = 0; i < 5; ++i)
                {
                    if (GUILayout.Button((i + 1).ToString()))
                        settings.selectionSets[i].selection = m_target.selection;
                }
                GUILayout.EndHorizontal();

                GUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Load", GUILayout.Width(50));
                for (int i = 0; i < 5; ++i)
                {
                    if (GUILayout.Button((i + 1).ToString()))
                        m_target.selection = settings.selectionSets[i].selection;
                }
                GUILayout.EndHorizontal();

                EditorGUILayout.Space();

                GUILayout.BeginHorizontal();
                GUILayout.Space(EditorGUI.indentLevel * indentSize);
                if (GUILayout.Button("Select All"))
                {
                    if (m_target.SelectAll())
                        m_target.UpdateSelection();
                }
                if (GUILayout.Button("Clear Selection"))
                {
                    if (m_target.ClearSelection())
                        m_target.UpdateSelection();
                }
                GUILayout.EndHorizontal();
            }
            else if (settings.editMode == EditMode.Brush)
            {
                settings.brushMode = (BrushMode)GUILayout.SelectionGrid((int)settings.brushMode, strBrushTypes, 4);
                EditorGUILayout.Space();
                settings.brushUseSelection = EditorGUILayout.Toggle("Mask With Selection", settings.brushUseSelection);
                settings.brushRadius = EditorGUILayout.Slider("Brush Radius", settings.brushRadius, 0.01f, 1.0f);
                settings.brushStrength = EditorGUILayout.Slider("Brush Strength", settings.brushStrength, -1.0f, 1.0f);
                settings.brushFalloff = EditorGUILayout.Slider("Brush Falloff", settings.brushFalloff, 0.0f, 2.0f);
                EditorGUILayout.Space();

                if (settings.brushMode == BrushMode.Paint)
                {
                    GUILayout.BeginHorizontal();
                    settings.primary = EditorGUILayout.ColorField(settings.primary, GUILayout.Width(35));
                    settings.primary = NormalPainter.ToColor(EditorGUILayout.Vector3Field("", NormalPainter.ToVector(settings.primary)));
                    settings.pickNormal = GUILayout.Toggle(settings.pickNormal, "Pick", "Button", GUILayout.Width(90));
                    GUILayout.EndHorizontal();
                }
                else if (settings.brushMode == BrushMode.Pinch)
                {
                    settings.brushPinchOffset = EditorGUILayout.Slider("Pinch Offset", settings.brushPinchOffset, 0.0f, 2.0f);
                    settings.brushPinchSharpness = EditorGUILayout.Slider("Pinch Sharpness", settings.brushPinchSharpness, 0.0f, 2.0f);
                }
            }
            else if (settings.editMode == EditMode.Assign)
            {
                settings.assignValue = EditorGUILayout.Vector3Field("Value", settings.assignValue);
                settings.assignLocal = EditorGUILayout.Toggle("Local Coordinate", settings.assignLocal);
                if (GUILayout.Button("Assign"))
                {
                    m_target.ApplyAssign(settings.assignValue, settings.assignLocal);
                    m_target.PushUndo();
                }
            }
            else if (settings.editMode == EditMode.Move)
            {
                settings.moveAmount = EditorGUILayout.Vector3Field("Move Amount", settings.moveAmount);
                settings.assignLocal = EditorGUILayout.Toggle("Local Coordinate", settings.assignLocal);
                EditorGUILayout.Space();
                settings.pivotPos = EditorGUILayout.Vector3Field("Pivot Position", settings.pivotPos);
                settings.pivotRot = Quaternion.Euler(EditorGUILayout.Vector3Field("Pivot Rotation", settings.pivotRot.eulerAngles));
                EditorGUILayout.Space();

                if (GUILayout.Button("Apply Move"))
                {
                    m_target.ApplyMove(settings.moveAmount, settings.assignLocal);
                    m_target.PushUndo();
                }
            }
            else if (settings.editMode == EditMode.Rotate)
            {
                settings.rotateAmount = EditorGUILayout.Vector3Field("Rotate Amount", settings.rotateAmount);
                settings.assignLocal = EditorGUILayout.Toggle("Local Coordinate", settings.assignLocal);
                EditorGUILayout.Space();
                settings.rotatePivot = EditorGUILayout.Toggle("Rotate Around Pivot", settings.rotatePivot);
                settings.pivotPos = EditorGUILayout.Vector3Field("Pivot Position", settings.pivotPos);
                settings.pivotRot = Quaternion.Euler(EditorGUILayout.Vector3Field("Pivot Rotation", settings.pivotRot.eulerAngles));
                EditorGUILayout.Space();

                if (GUILayout.Button("Apply Rotate"))
                {
                    if (settings.rotatePivot)
                        m_target.ApplyRotatePivot(
                            Quaternion.Euler(settings.rotateAmount), settings.pivotPos, settings.pivotRot, settings.assignLocal);
                    else
                        m_target.ApplyRotate(Quaternion.Euler(settings.rotateAmount), settings.pivotRot, settings.assignLocal);
                    m_target.PushUndo();
                }
            }
            else if (settings.editMode == EditMode.Scale)
            {
                settings.scaleAmount = EditorGUILayout.Vector3Field("Scale Amount", settings.scaleAmount);
                settings.assignLocal = EditorGUILayout.Toggle("Local Coordinate", settings.assignLocal);
                EditorGUILayout.Space();
                settings.pivotPos = EditorGUILayout.Vector3Field("Pivot Position", settings.pivotPos);
                settings.pivotRot = Quaternion.Euler(EditorGUILayout.Vector3Field("Pivot Rotation", settings.pivotRot.eulerAngles));
                EditorGUILayout.Space();
                if (GUILayout.Button("Apply Scale"))
                {
                    m_target.ApplyScale(settings.scaleAmount, settings.pivotPos, settings.pivotRot, settings.assignLocal);
                    m_target.PushUndo();
                }
            }
            else if (settings.editMode == EditMode.Equalize)
            {
                settings.equalizeRadius = EditorGUILayout.FloatField("Equalize Radius", settings.equalizeRadius);
                settings.equalizeAmount = EditorGUILayout.FloatField("Equalize Amount", settings.equalizeAmount);
                if (GUILayout.Button("Apply Equalize"))
                {
                    m_target.ApplyEqualize(settings.equalizeRadius, settings.equalizeAmount);
                    m_target.PushUndo();
                }
            }
            else if (settings.editMode == EditMode.Projection)
            {
                settings.projector = EditorGUILayout.ObjectField("Projector", settings.projector, typeof(GameObject), true) as GameObject;
                if (GUILayout.Button("Apply Projection"))
                {
                    m_target.ApplyProjection(settings.projector);
                    m_target.PushUndo();
                }
            }
            else if (settings.editMode == EditMode.Reset)
            {
                EditorGUILayout.BeginHorizontal();
                if (GUILayout.Button("Reset (Selection)"))
                {
                    m_target.ResetNormals(true);
                    m_target.PushUndo();
                }
                else if (GUILayout.Button("Reset (All)"))
                {
                    m_target.ResetNormals(false);
                    m_target.PushUndo();
                }
                EditorGUILayout.EndHorizontal();
            }
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();
        }

        void DrawMiscPanel()
        {
            var settings = m_target.settings;

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(indentSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(c1Width));
            EditorGUILayout.LabelField("", GUILayout.Width(c1Width));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(spaceSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();
            {
                var mirrorMode = settings.mirrorMode;
                settings.mirrorMode = (MirrorMode)EditorGUILayout.EnumPopup("Mirroring", settings.mirrorMode);
                if (mirrorMode != settings.mirrorMode)
                {
                    if (m_target.ApplyMirroring())
                    {
                        m_target.UpdateNormals();
                        m_target.PushUndo();
                    }
                }

                EditorGUILayout.Space();
                if (GUILayout.Button("Recalculate Tangents"))
                    m_target.RecalculateTangents();
            }
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();
        }


        static readonly string[] strInExport = new string[] {
            "Vertex Color",
            "Bake Texture",
            "Load Texture",
            "Export .asset",
            "Export .obj",
        };

        void DrawInExportPanel()
        {
            var settings = m_target.settings;

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(indentSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(c1Width));
            settings.inexportIndex = GUILayout.SelectionGrid(settings.inexportIndex, strInExport, 1);
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(spaceSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();

            if (settings.inexportIndex == 0)
            {
                GUILayout.BeginHorizontal();
                if (GUILayout.Button("Convert To Vertex Color"))
                    m_target.BakeToVertexColor();
                if (GUILayout.Button("Convert From Vertex Color"))
                    m_target.LoadVertexColor();
                GUILayout.EndHorizontal();
            }
            else if (settings.inexportIndex == 1)
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
            else if (settings.inexportIndex == 2)
            {
                settings.bakeSource = EditorGUILayout.ObjectField("Source Texture", settings.bakeSource, typeof(Texture), true) as Texture;

                if (GUILayout.Button("Load"))
                    m_target.LoadTexture(settings.bakeSource);
            }
            else if (settings.inexportIndex == 3)
            {
                if (GUILayout.Button("Export .asset file"))
                {
                    string path = EditorUtility.SaveFilePanel("Export .asset file", "Assets", m_target.name, "asset");
                    if (path.Length > 0)
                    {
                        var dataPath = Application.dataPath;
                        if (!path.StartsWith(dataPath))
                        {
                            Debug.LogError("Invalid path: Path must be under " + dataPath);
                        }
                        else
                        {
                            path = path.Replace(dataPath, "Assets");
                            AssetDatabase.CreateAsset(Instantiate(m_target.mesh), path);
                            Debug.Log("Asset exported: " + path);
                        }
                    }
                }
            }
            else if (settings.inexportIndex == 4)
            {
                settings.objFlipHandedness = EditorGUILayout.Toggle("Flip Handedness", settings.objFlipHandedness);
                settings.objFlipFaces = EditorGUILayout.Toggle("Flip Faces", settings.objFlipFaces);
                settings.objMakeSubmeshes = EditorGUILayout.Toggle("Make Submeshes", settings.objMakeSubmeshes);
                settings.objApplyTransform = EditorGUILayout.Toggle("Apply Transform", settings.objApplyTransform);
                settings.objIncludeChildren = EditorGUILayout.Toggle("Include Children", settings.objIncludeChildren);

                if (GUILayout.Button("Export .obj file"))
                {
                    string path = EditorUtility.SaveFilePanel("Export .obj file", "", m_target.name, "obj");
                    ObjExporter.Export(m_target.gameObject, path, new ObjExporter.Settings
                    {
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


        static readonly string[] strDisplay = new string[] {
            "Display",
            "Options",
        };

        void DrawDisplayPanel()
        {
            var settings = m_target.settings;

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(indentSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(c1Width));
            settings.displayIndex = GUILayout.SelectionGrid(settings.displayIndex, strDisplay, 1);
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(spaceSize));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();
            if (settings.displayIndex == 0)
            {
                settings.showVertices = EditorGUILayout.Toggle("Vertices", settings.showVertices);
                settings.showNormals = EditorGUILayout.Toggle("Normals", settings.showNormals);
                settings.showTangents = EditorGUILayout.Toggle("Tangents", settings.showTangents);
                settings.showBinormals = EditorGUILayout.Toggle("Binormals", settings.showBinormals);
                EditorGUI.indentLevel++;
                settings.showSelectedOnly = EditorGUILayout.Toggle("Only Selected", settings.showSelectedOnly);
                EditorGUI.indentLevel--;
                settings.modelOverlay = (ModelOverlay)EditorGUILayout.EnumPopup("Overlay", settings.modelOverlay);
            }
            else if (settings.displayIndex == 1)
            {
                settings.vertexSize = EditorGUILayout.Slider("Vertex Size", settings.vertexSize, 0.0f, 0.05f);
                settings.normalSize = EditorGUILayout.Slider("Normal Size", settings.normalSize, 0.0f, 1.00f);
                settings.tangentSize = EditorGUILayout.Slider("Tangent Size", settings.tangentSize, 0.0f, 1.00f);
                settings.binormalSize = EditorGUILayout.Slider("Binormal Size", settings.binormalSize, 0.0f, 1.00f);

                EditorGUILayout.Space();

                settings.vertexColor = EditorGUILayout.ColorField("Vertex Color", settings.vertexColor);
                settings.vertexColor2 = EditorGUILayout.ColorField("Vertex Color (Selected)", settings.vertexColor2);
                settings.vertexColor3 = EditorGUILayout.ColorField("Vertex Color (Highlighted)", settings.vertexColor3);
                settings.normalColor = EditorGUILayout.ColorField("Normal Color", settings.normalColor);
                settings.tangentColor = EditorGUILayout.ColorField("Tangent Color", settings.tangentColor);
                settings.binormalColor = EditorGUILayout.ColorField("Binormal Color", settings.binormalColor);
                if (GUILayout.Button("Reset"))
                {
                    settings.ResetDisplayOptions();
                }
            }
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();
        }


        void DrawNormalPainter()
        {
            var settings = m_target.settings;

            EditorGUI.BeginChangeCheck();

            EditorGUILayout.Space();

            settings.foldEdit = EditorGUILayout.Foldout(settings.foldEdit, "Edit");
            if (settings.foldEdit)
                DrawEditPanel();

            settings.foldMisc = EditorGUILayout.Foldout(settings.foldEdit, "Misc");
            if (settings.foldMisc)
                DrawMiscPanel();

            EditorGUILayout.Space();

            settings.foldInExport = EditorGUILayout.Foldout(settings.foldInExport, "Import / Export");
            if (settings.foldInExport)
                DrawInExportPanel();

            EditorGUILayout.Space();

            settings.foldDisplay = EditorGUILayout.Foldout(settings.foldDisplay, "Display");
            if (settings.foldDisplay)
                DrawDisplayPanel();

            if (EditorGUI.EndChangeCheck())
                RepaintAllViews();
        }


        bool HandleShortcutKeys()
        {
            bool handled = false;
            var settings = m_target.settings;
            var e = Event.current;

            if (e.type == EventType.KeyDown)
            {
                var prevEditMode = settings.editMode;
                switch (e.keyCode)
                {
                    case KeyCode.F1: settings.editMode = EditMode.Select; break;
                    case KeyCode.F2: settings.editMode = EditMode.Brush; break;
                    case KeyCode.F3: settings.editMode = EditMode.Assign; break;
                    case KeyCode.F4: settings.editMode = EditMode.Move; break;
                    case KeyCode.F5: settings.editMode = EditMode.Rotate; break;
                    case KeyCode.F6: settings.editMode = EditMode.Scale; break;
                    case KeyCode.F7: settings.editMode = EditMode.Equalize; break;
                    case KeyCode.F8: settings.editMode = EditMode.Projection; break;
                    case KeyCode.F9: settings.editMode = EditMode.Reset; break;
                }
                if (settings.editMode != prevEditMode)
                    handled = true;

                if (settings.editMode == EditMode.Select)
                {
                    var prevSelectMode = settings.selectMode;
                    switch (e.keyCode)
                    {
                        case KeyCode.Alpha1: settings.selectMode = SelectMode.Single; break;
                        case KeyCode.Alpha2: settings.selectMode = SelectMode.Rect; break;
                        case KeyCode.Alpha3: settings.selectMode = SelectMode.Lasso; break;
                        case KeyCode.Alpha4: settings.selectMode = SelectMode.Brush; break;
                    }
                    if (settings.selectMode != prevSelectMode)
                        handled = true;
                }
                else if (settings.editMode == EditMode.Brush)
                {
                    var prevBrushMode = settings.brushMode;
                    switch (e.keyCode)
                    {
                        case KeyCode.Alpha1: settings.brushMode = BrushMode.Paint; break;
                        case KeyCode.Alpha2: settings.brushMode = BrushMode.Pinch; break;
                        case KeyCode.Alpha3: settings.brushMode = BrushMode.Equalize; break;
                        case KeyCode.Alpha4: settings.brushMode = BrushMode.Reset; break;
                    }
                    if (settings.brushMode != prevBrushMode)
                        handled = true;

                    if (settings.brushMode == BrushMode.Paint)
                    {
                        if(e.keyCode == KeyCode.P)
                        {
                            settings.pickNormal = true;
                            handled = true;
                        }
                    }
                }

                if (e.keyCode == KeyCode.A)
                {
                    m_target.SelectAll();
                    m_target.UpdateSelection();
                    handled = true;
                }
                else if (e.keyCode == KeyCode.C)
                {
                    m_target.ClearSelection();
                    m_target.UpdateSelection();
                    handled = true;
                }
                else if (e.keyCode == KeyCode.T)
                {
                    m_target.RecalculateTangents();
                    handled = true;
                }
            }
            else if (e.type == EventType.ScrollWheel)
            {
                if (settings.editMode == EditMode.Brush ||
                    (settings.editMode == EditMode.Select && settings.selectMode == SelectMode.Brush))
                {
                    if (e.shift)
                    {
                        settings.brushRadius = Mathf.Clamp(settings.brushRadius + -e.delta.y * 0.01f, 0.01f, 2.0f);
                        handled = true;
                    }
                    else if (e.control)
                    {
                        settings.brushStrength = Mathf.Clamp(settings.brushStrength + -e.delta.y * 0.02f, -1.0f, 1.0f);
                        handled = true;
                    }
                    else if (e.alt)
                    {
                        settings.brushFalloff = Mathf.Clamp(settings.brushFalloff + -e.delta.y * 0.02f, 0.0f, 2.0f);
                        handled = true;
                    }
                }
            }

            return handled;
        }
    }
}
