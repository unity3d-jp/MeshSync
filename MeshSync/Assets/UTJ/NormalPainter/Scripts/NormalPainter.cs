using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.NormalPainter
{
    public enum EditMode
    {
        Select,
        Brush,
        Assign,
        Move,
        Rotate,
        Scale,
        Equalize,
        Projection,
        Reset,
    }

    public enum BrushMode
    {
        Paint,
        Pinch,
        Equalize,
        Reset,
    }

    public enum SelectMode
    {
        Single,
        Rect,
        Lasso,
        Brush,
    }

    public enum MirrorMode
    {
        None,
        RightToLeft,
        LeftToRight,
        ForwardToBack,
        BackToForward,
        UpToDown,
        DownToUp,
    }

    public enum ImageFormat
    {
        PNG,
        EXR,
    }

    public enum ModelOverlay
    {
        None,
        LocalSpaceNormals,
        TangentSpaceNormals,
        VertexColor,
    }

    public enum SceneGUIState
    {
        Repaint = 1 << 0,
        SelectionChanged = 1 << 1,
    }



    [ExecuteInEditMode]
    public partial class NormalPainter : MonoBehaviour
    {
#if UNITY_EDITOR

        [Serializable]
        class History
        {
            public int count = 0;
            public Vector3[] normals;
        }


        NormalPainterSettings m_settings;
        
        // internal resources
        [SerializeField] Mesh m_meshTarget;
        [SerializeField] Mesh m_meshCube;
        [SerializeField] Mesh m_meshLine;
        [SerializeField] Mesh m_meshLasso;
        [SerializeField] Material m_matVisualize;
        [SerializeField] Material m_matBake;
        [SerializeField] ComputeShader m_csBakeFromMap;

        ComputeBuffer m_cbArg;
        ComputeBuffer m_cbPoints;
        ComputeBuffer m_cbNormals;
        ComputeBuffer m_cbTangents;
        ComputeBuffer m_cbSelection;
        ComputeBuffer m_cbBaseNormals;
        ComputeBuffer m_cbBaseTangents;
        CommandBuffer m_cmdDraw;

        Vector3[]   m_points;
        Vector3[]   m_normals;
        Vector4[]   m_tangents;
        Vector3[]   m_baseNormals;
        Vector4[]   m_baseTangents;
        int[]       m_triangles;
        int[]       m_mirrorRelation;
        float[]     m_selection;

        int         m_numSelected = 0;
        bool        m_rayHit;
        int         m_rayHitTriangle;
        Vector3     m_rayPos;
        Vector3     m_rayNormal;
        Vector3     m_selectionPos;
        Vector3     m_selectionNormal;
        Quaternion  m_selectionRot;
        bool        m_rectDragging;
        Vector2     m_rectStartPoint;
        Vector2     m_rectEndPoint;
        List<Vector2> m_lassoPoints = new List<Vector2>();
        int         m_brushNumPainted = 0;

        [SerializeField] History m_history = new History();


        public NormalPainterSettings settings { get { return m_settings; } }
        public Mesh mesh { get { return m_meshTarget; } }

        public float[] selection
        {
            get { return (float[])m_selection.Clone(); }
            set
            {
                if (value != null && value.Length == m_selection.Length)
                {
                    Array.Copy(value, m_selection, m_selection.Length);
                    UpdateSelection();
                }
            }
        }


        Mesh GetTargetMesh()
        {
            Mesh ret = null;
            {
                var mf = GetComponent<MeshFilter>();
                if (mf != null) { ret = mf.sharedMesh; }
            }
            if (ret == null)
            {
                var smr = GetComponent<SkinnedMeshRenderer>();
                if(smr != null) { ret = smr.sharedMesh; }
            }
            return ret;
        }

        void SetupResources()
        {
            if (m_settings == null)
                m_settings = ScriptableObject.CreateInstance<NormalPainterSettings>();

            if (m_meshCube == null)
            {
                float l = 0.5f;
                var p = new Vector3[] {
                    new Vector3(-l,-l, l),
                    new Vector3( l,-l, l),
                    new Vector3( l,-l,-l),
                    new Vector3(-l,-l,-l),

                    new Vector3(-l, l, l),
                    new Vector3( l, l, l),
                    new Vector3( l, l,-l),
                    new Vector3(-l, l,-l),
                };

                m_meshCube = new Mesh();
                m_meshCube.vertices = new Vector3[] {
                    p[0], p[1], p[2], p[3],
                    p[7], p[4], p[0], p[3],
                    p[4], p[5], p[1], p[0],
                    p[6], p[7], p[3], p[2],
                    p[5], p[6], p[2], p[1],
                    p[7], p[6], p[5], p[4],
                };
                m_meshCube.SetIndices(new int[] {
                    3, 1, 0, 3, 2, 1,
                    7, 5, 4, 7, 6, 5,
                    11, 9, 8, 11, 10, 9,
                    15, 13, 12, 15, 14, 13,
                    19, 17, 16, 19, 18, 17,
                    23, 21, 20, 23, 22, 21,
                }, MeshTopology.Triangles, 0);
            }

            if (m_meshLine == null)
            {
                m_meshLine = new Mesh();
                m_meshLine.vertices = new Vector3[2] { Vector3.zero, Vector3.zero };
                m_meshLine.uv = new Vector2[2] { Vector2.zero, Vector2.one };
                m_meshLine.SetIndices(new int[2] { 0, 1 }, MeshTopology.Lines, 0);
            }

            if (m_meshLasso == null)
            {
                m_meshLasso = new Mesh();
            }

            if (m_matVisualize == null)
                m_matVisualize = new Material(AssetDatabase.LoadAssetAtPath<Shader>(AssetDatabase.GUIDToAssetPath("03871fa9be0375f4c91cb4842f15b890")));
            if (m_matBake == null)
                m_matBake = new Material(AssetDatabase.LoadAssetAtPath<Shader>(AssetDatabase.GUIDToAssetPath("4ddd0053dc720414b8afc76bf0a93f8e")));
            if (m_csBakeFromMap == null)
                m_csBakeFromMap = AssetDatabase.LoadAssetAtPath<ComputeShader>(AssetDatabase.GUIDToAssetPath("f6687b99e1b6bfc4f854f46669e84e31"));

            var tmesh = GetTargetMesh();
            if (tmesh == null) { return; }

            if (m_meshTarget == null ||
                m_meshTarget.vertexCount != tmesh.vertexCount ||
                (m_points != null && m_meshTarget.vertexCount != m_points.Length))
            {
                m_meshTarget = tmesh;
                m_points = null;
                m_normals = null;
                m_baseNormals = null;
                m_tangents = null;
                m_triangles = null;
                m_mirrorRelation = null;
                m_selection = null;
                ReleaseComputeBuffers();
            }

            bool initialized = false;
            if (m_points == null && m_meshTarget != null)
            {
                initialized = true;
                m_points = m_meshTarget.vertices;

                m_normals = m_meshTarget.normals;
                if (m_normals.Length == 0)
                {
                    m_meshTarget.RecalculateNormals();
                    m_baseNormals = m_normals = m_meshTarget.normals;
                }
                else
                {
                    m_meshTarget.RecalculateNormals();
                    m_baseNormals = m_meshTarget.normals;
                    m_meshTarget.normals = m_normals;
                }

                m_tangents = m_meshTarget.tangents;
                if (m_tangents.Length == 0)
                {
                    m_meshTarget.RecalculateNormals();
                    m_baseTangents = m_tangents = m_meshTarget.tangents;
                }
                else
                {
                    m_meshTarget.RecalculateNormals();
                    m_baseTangents = m_meshTarget.tangents;
                    m_meshTarget.tangents = m_tangents;
                }

                m_triangles = m_meshTarget.triangles;
                m_selection = new float[m_points.Length];
            }

            if (m_cbPoints == null && m_points != null && m_points.Length > 0)
            {
                m_cbPoints = new ComputeBuffer(m_points.Length, 12);
                m_cbPoints.SetData(m_points);
            }
            if (m_cbNormals == null && m_normals != null && m_normals.Length > 0)
            {
                m_cbNormals = new ComputeBuffer(m_normals.Length, 12);
                m_cbNormals.SetData(m_normals);
                m_cbBaseNormals = new ComputeBuffer(m_baseNormals.Length, 12);
                m_cbBaseNormals.SetData(m_baseNormals);
            }
            if (m_cbTangents == null && m_tangents != null && m_tangents.Length > 0)
            {
                m_cbTangents = new ComputeBuffer(m_tangents.Length, 16);
                m_cbTangents.SetData(m_tangents);
                m_cbBaseTangents = new ComputeBuffer(m_baseTangents.Length, 16);
                m_cbBaseTangents.SetData(m_baseTangents);
            }
            if (m_cbSelection == null && m_selection != null && m_selection.Length > 0)
            {
                m_cbSelection = new ComputeBuffer(m_selection.Length, 4);
                m_cbSelection.SetData(m_selection);
            }

            if (m_cbArg == null && m_points != null && m_points.Length > 0)
            {
                m_cbArg = new ComputeBuffer(1, 5 * sizeof(uint), ComputeBufferType.IndirectArguments);
                m_cbArg.SetData(new uint[5] { m_meshCube.GetIndexCount(0), (uint)m_points.Length, 0, 0, 0 });
            }

            if (initialized)
            {
                UpdateNormals();
                PushUndo();
            }
        }

        void ReleaseComputeBuffers()
        {
            if (m_cbArg != null) { m_cbArg.Release(); m_cbArg = null; }
            if (m_cbPoints != null) { m_cbPoints.Release(); m_cbPoints = null; }
            if (m_cbNormals != null) { m_cbNormals.Release(); m_cbNormals = null; }
            if (m_cbTangents != null) { m_cbTangents.Release(); m_cbTangents = null; }
            if (m_cbSelection != null) { m_cbSelection.Release(); m_cbSelection = null; }
            if (m_cbBaseNormals != null) { m_cbBaseNormals.Release(); m_cbBaseNormals = null; }
            if (m_cbBaseTangents != null) { m_cbBaseTangents.Release(); m_cbBaseTangents = null; }
            if (m_cmdDraw != null) { m_cmdDraw.Release(); m_cmdDraw = null; }
        }


        void OnEnable()
        {
            Undo.undoRedoPerformed += OnUndoRedo;
            SetupResources();
        }

        void OnDisable()
        {
            ReleaseComputeBuffers();
            Undo.undoRedoPerformed -= OnUndoRedo;
        }

        public int OnSceneGUI()
        {
            if (!isActiveAndEnabled || m_points == null)
                return 0;

            int ret = 0;
            ret |= HandleEditTools();

            Event e = Event.current;
            var et = e.type;
            int id = GUIUtility.GetControlID(FocusType.Passive);
            et = e.GetTypeForControl(id);

            if ((et == EventType.MouseDown || et == EventType.MouseDrag || et == EventType.MouseUp) && e.button == 0)
            {
                ret |= HandleMouseEvent(e, et, id);
            }

            if (Event.current.type == EventType.Repaint)
                OnRepaint();
            return ret;
        }


        Vector3 m_prevMove;
        Quaternion m_prevRot;
        Vector3 m_prevScale;

        public int HandleEditTools()
        {
            var editMode = m_settings.editMode;
            Event e = Event.current;
            var et = e.type;
            int ret = 0;
            bool handled = false;
            var t = GetComponent<Transform>();

            if (et == EventType.MouseMove || et == EventType.MouseDrag)
            {
                bool prevRayHit = m_rayHit;
                m_rayHit = Raycast(e, ref m_rayPos, ref m_rayHitTriangle);
                if (m_rayHit || prevRayHit)
                    ret |= (int)SceneGUIState.Repaint;
            }

            if (m_numSelected > 0 && editMode == EditMode.Move)
            {
                var pivotRot = m_settings.assignLocal ? t.rotation : m_settings.pivotRot;
                if (et == EventType.MouseDown)
                    m_prevMove = m_settings.pivotPos;

                EditorGUI.BeginChangeCheck();
                var move = Handles.PositionHandle(m_settings.pivotPos, pivotRot);
                if (EditorGUI.EndChangeCheck())
                {
                    handled = true;
                    var diff = move - m_prevMove;
                    m_prevMove = move;
                    ApplyMove(diff * 3.0f);
                }
            }
            else if (m_numSelected > 0 && editMode == EditMode.Rotate)
            {
                var pivotRot = m_settings.assignLocal ? t.rotation : m_settings.pivotRot;
                if (et == EventType.MouseDown)
                    m_prevRot = pivotRot;

                EditorGUI.BeginChangeCheck();
                var rot = Handles.RotationHandle(pivotRot, m_settings.pivotPos);
                if (EditorGUI.EndChangeCheck())
                {
                    handled = true;
                    var diff = Quaternion.Inverse(m_prevRot) * rot;
                    m_prevRot = rot;
                    if (m_settings.rotatePivot)
                        ApplyRotatePivot(diff, m_settings.pivotPos, pivotRot);
                    else
                        ApplyRotate(diff, pivotRot);
                }
            }
            else if (m_numSelected > 0 && editMode == EditMode.Scale)
            {
                var pivotRot = m_settings.assignLocal ? t.rotation : m_settings.pivotRot;
                if (et == EventType.MouseDown)
                    m_prevScale = Vector3.one;

                EditorGUI.BeginChangeCheck();
                var scale = Handles.ScaleHandle(Vector3.one, m_settings.pivotPos,
                    pivotRot, HandleUtility.GetHandleSize(m_settings.pivotPos));
                if (EditorGUI.EndChangeCheck())
                {
                    handled = true;
                    var diff = scale - m_prevScale;
                    m_prevScale = scale;
                    ApplyScale(diff, m_settings.pivotPos, pivotRot);
                }
            }

            if (handled)
            {
                ret |= (int)SceneGUIState.Repaint;
                if (et == EventType.MouseUp || et == EventType.MouseMove)
                    PushUndo();
            }
            return ret;
        }

        public static Color ToColor(Vector3 n)
        {
            return new Color(n.x * 0.5f + 0.5f, n.y * 0.5f + 0.5f, n.z * 0.5f + 0.5f, 1.0f);
        }
        public static Vector3 ToVector(Color n)
        {
            return new Vector3(n.r * 2.0f - 1.0f, n.g * 2.0f - 1.0f, n.b * 2.0f - 1.0f);
        }

        int HandleMouseEvent(Event e, EventType et, int id)
        {
            int ret = 0;
            var editMode = m_settings.editMode;
            bool handled = false;

            if (editMode == EditMode.Brush && m_settings.pickNormal)
            {
                if (m_rayHit)
                {
                    m_settings.primary = ToColor(PickNormal(m_rayPos, m_rayHitTriangle));
                    handled = true;
                }
                m_settings.pickNormal = false;
            }
            else if (editMode == EditMode.Brush)
            {
                if (m_rayHit && (et == EventType.MouseDown || et == EventType.MouseDrag))
                {
                    switch (m_settings.brushMode)
                    {
                        case BrushMode.Paint:
                            if (ApplyAdditiveBrush(m_settings.brushUseSelection, m_rayPos, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength,
                                ToVector(m_settings.primary).normalized))
                                ++m_brushNumPainted;
                            break;
                        case BrushMode.Pinch:
                            if (ApplyPinchBrush(m_settings.brushUseSelection, m_rayPos, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength,
                                PickBaseNormal(m_rayPos, m_rayHitTriangle), m_settings.brushPinchOffset, m_settings.brushPinchSharpness))
                                ++m_brushNumPainted;
                            break;
                        case BrushMode.Equalize:
                            if (ApplyEqualizeBrush(m_settings.brushUseSelection, m_rayPos, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength))
                                ++m_brushNumPainted;
                            break;
                        case BrushMode.Reset:
                            if (ApplyResetBrush(m_settings.brushUseSelection, m_rayPos, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength))
                                ++m_brushNumPainted;
                            break;
                    }
                    handled = true;
                }

                if (et == EventType.MouseUp)
                {
                    if (m_brushNumPainted > 0)
                    {
                        PushUndo();
                        m_brushNumPainted = 0;
                        handled = true;
                    }
                }
            }
            else
            {
                var selectMode = m_settings.selectMode;
                float selectSign = e.control ? -1.0f : 1.0f;

                if (selectMode == SelectMode.Single)
                {
                    if (!e.shift && !e.control)
                        System.Array.Clear(m_selection, 0, m_selection.Length);

                    if (SelectSingle(e, selectSign, settings.selectFrontSideOnly) || m_rayHit)
                    {
                        handled = true;
                    }
                }
                else if (selectMode == SelectMode.Rect)
                {
                    if (et == EventType.MouseDown)
                    {
                        m_rectDragging = true;
                        m_rectStartPoint = m_rectEndPoint = e.mousePosition;
                        handled = true;
                    }
                    else if (et == EventType.MouseDrag)
                    {
                        m_rectEndPoint = e.mousePosition;
                        handled = true;
                    }
                    else if (et == EventType.MouseUp)
                    {
                        m_rectDragging = false;
                        if (!e.shift && !e.control)
                            System.Array.Clear(m_selection, 0, m_selection.Length);

                        m_rectEndPoint = e.mousePosition;
                        handled = true;

                        if (!SelectRect(m_rectStartPoint, m_rectEndPoint, selectSign, settings.selectFrontSideOnly) && !m_rayHit)
                        {
                        }
                        m_rectStartPoint = m_rectEndPoint = -Vector2.one;
                    }
                }
                else if (selectMode == SelectMode.Lasso)
                {
                    if (et == EventType.MouseDown || et == EventType.MouseDrag)
                    {
                        if(et == EventType.MouseDown)
                        {
                            m_lassoPoints.Clear();
                            m_meshLasso.Clear();
                        }

                        m_lassoPoints.Add(ScreenCoord11(e.mousePosition));
                        handled = true;

                        m_meshLasso.Clear();
                        if (m_lassoPoints.Count > 1)
                        {
                            var vertices = new Vector3[m_lassoPoints.Count];
                            var indices = new int[(vertices.Length - 1) * 2];
                            for (int i = 0; i < vertices.Length; ++i)
                            {
                                vertices[i].x = m_lassoPoints[i].x;
                                vertices[i].y = m_lassoPoints[i].y;
                            }
                            for (int i = 0; i < vertices.Length - 1; ++i)
                            {
                                indices[i * 2 + 0] = i;
                                indices[i * 2 + 1] = i + 1;
                            }
                            m_meshLasso.vertices = vertices;
                            m_meshLasso.SetIndices(indices, MeshTopology.Lines, 0);
                        }
                    }
                    else if (et == EventType.MouseUp)
                    {
                        if (!e.shift && !e.control)
                            System.Array.Clear(m_selection, 0, m_selection.Length);

                        handled = true;
                        if (!SelectLasso(m_lassoPoints.ToArray(), selectSign, settings.selectFrontSideOnly) && !m_rayHit)
                        {
                        }

                        m_lassoPoints.Clear();
                        m_meshLasso.Clear();
                    }
                }
                else if (selectMode == SelectMode.Brush)
                {
                    if (et == EventType.MouseDown && !e.shift && !e.control)
                        System.Array.Clear(m_selection, 0, m_selection.Length);

                    if (et == EventType.MouseDown || et == EventType.MouseDrag)
                    {
                        if (m_rayHit && SelectSoft(m_rayPos, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength * selectSign))
                            handled = true;
                    }
                }

                UpdateSelection();
            }

            if (et == EventType.MouseDown)
            {
                GUIUtility.hotControl = id;
            }
            else if (et == EventType.MouseUp)
            {
                if (GUIUtility.hotControl == id && e.button == 0)
                    GUIUtility.hotControl = 0;
            }
            e.Use();

            if (handled)
            {
                ret |= (int)SceneGUIState.Repaint;
            }
            return ret;
        }


        void OnDrawGizmosSelected()
        {
            SetupResources();
            if(!m_settings.editing) { return; }

            if (m_matVisualize == null || m_meshCube == null || m_meshLine == null)
            {
                Debug.LogWarning("NormalEditor: Some resources are missing.\n");
                return;
            }

            var trans = GetComponent<Transform>();
            var matrix = trans.localToWorldMatrix;

            m_matVisualize.SetMatrix("_Transform", matrix);
            m_matVisualize.SetFloat("_VertexSize", m_settings.vertexSize);
            m_matVisualize.SetFloat("_NormalSize", m_settings.normalSize);
            m_matVisualize.SetFloat("_TangentSize", m_settings.tangentSize);
            m_matVisualize.SetFloat("_BinormalSize", m_settings.binormalSize);
            m_matVisualize.SetColor("_VertexColor", m_settings.vertexColor);
            m_matVisualize.SetColor("_VertexColor2", m_settings.vertexColor2);
            m_matVisualize.SetColor("_VertexColor3", m_settings.vertexColor3);
            m_matVisualize.SetColor("_NormalColor", m_settings.normalColor);
            m_matVisualize.SetColor("_TangentColor", m_settings.tangentColor);
            m_matVisualize.SetColor("_BinormalColor", m_settings.binormalColor);
            m_matVisualize.SetInt("_OnlySelected", m_settings.showSelectedOnly ? 1 : 0);
            if (m_rayHit && 
                (m_settings.editMode == EditMode.Brush ||
                 (m_settings.editMode == EditMode.Select && m_settings.selectMode == SelectMode.Brush)))
            {
                m_matVisualize.SetVector("_RayPos", m_rayPos);
                m_matVisualize.SetVector("_RayRadPow", new Vector3(m_settings.brushRadius, m_settings.brushFalloff, 0.0f));
            }
            else
            {
                m_matVisualize.SetVector("_RayRadPow", new Vector3(0.0f, 1.0f, 0.0f));
            }
            if (m_cbPoints != null) m_matVisualize.SetBuffer("_Points", m_cbPoints);
            if (m_cbNormals != null) m_matVisualize.SetBuffer("_Normals", m_cbNormals);
            if (m_cbTangents != null) m_matVisualize.SetBuffer("_Tangents", m_cbTangents);
            if (m_cbSelection != null) m_matVisualize.SetBuffer("_Selection", m_cbSelection);
            if (m_cbBaseNormals != null) m_matVisualize.SetBuffer("_BaseNormals", m_cbBaseNormals);
            if (m_cbBaseTangents != null) m_matVisualize.SetBuffer("_BaseTangents", m_cbBaseTangents);

            if (m_cmdDraw == null)
            {
                m_cmdDraw = new CommandBuffer();
                m_cmdDraw.name = "NormalEditor";
            }
            m_cmdDraw.Clear();

            switch (m_settings.modelOverlay)
            {
                case ModelOverlay.LocalSpaceNormals:
                    for (int si = 0; si < m_meshTarget.subMeshCount; ++si)
                        m_cmdDraw.DrawMesh(m_meshTarget, matrix, m_matVisualize, si, 4);
                    break;
                case ModelOverlay.TangentSpaceNormals:
                    for (int si = 0; si < m_meshTarget.subMeshCount; ++si)
                        m_cmdDraw.DrawMesh(m_meshTarget, matrix, m_matVisualize, si, 5);
                    break;
                case ModelOverlay.VertexColor:
                    for (int si = 0; si < m_meshTarget.subMeshCount; ++si)
                        m_cmdDraw.DrawMesh(m_meshTarget, matrix, m_matVisualize, si, 6);
                    break;
            }
            if (m_settings.showVertices && m_points != null)
                m_cmdDraw.DrawMeshInstancedIndirect(m_meshCube, 0, m_matVisualize, 0, m_cbArg);
            if (m_settings.showBinormals && m_tangents != null)
                m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_matVisualize, 3, m_cbArg);
            if (m_settings.showTangents && m_tangents != null)
                m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_matVisualize, 2, m_cbArg);
            if (m_settings.showNormals && m_normals != null)
                m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_matVisualize, 1, m_cbArg);
            if (m_meshLasso.vertexCount > 1)
                m_cmdDraw.DrawMesh(m_meshLasso, Matrix4x4.identity, m_matVisualize, 0, 7);
            Graphics.ExecuteCommandBuffer(m_cmdDraw);
        }

        void OnRepaint()
        {
            if (m_settings.selectMode == SelectMode.Rect && m_rectDragging)
            {
                var selectionRect = typeof(EditorStyles).GetProperty("selectionRect", BindingFlags.NonPublic | BindingFlags.Static);
                if (selectionRect != null)
                {
                    var style = (GUIStyle)selectionRect.GetValue(null, null);
                    Handles.BeginGUI();
                    style.Draw(FromToRect(m_rectStartPoint, m_rectEndPoint), GUIContent.none, false, false, false, false);
                    Handles.EndGUI();
                }
            }
        }

#endif
    }
}

