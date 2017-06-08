using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace UTJ.HumbleNormalEditor
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
        Scale,
        Equalize,
        Reset,
    }

    public enum SelectMode
    {
        Single,
        Rect,
        Brush,
        //Lasso,
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
    [RequireComponent(typeof(MeshRenderer))]
    public partial class NormalEditor : MonoBehaviour
    {
#if UNITY_EDITOR

        [Serializable]
        class History
        {
            public int count = 0;
            public Vector3[] normals;
        }


        NormalEditorSettings m_settings;
        
        // internal resources
        [SerializeField] Mesh m_meshTarget;
        [SerializeField] Mesh m_meshCube;
        [SerializeField] Mesh m_meshLine;
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
        Vector3     m_rayPos;
        Vector3     m_selectionPos;
        Vector3     m_selectionNormal;
        Quaternion  m_selectionRot;
        Vector2     m_dragStartPoint;
        Vector2     m_dragEndPoint;

        [SerializeField] History m_history = new History();


        public NormalEditorSettings settings { get { return m_settings; } }
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



        void SetupResources()
        {
            if (m_settings == null)
                m_settings = ScriptableObject.CreateInstance<NormalEditorSettings>();

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

            if (m_matVisualize == null)
                m_matVisualize = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/NormalEditor/Shaders/Visualizer.shader"));
            if (m_matBake == null)
                m_matBake = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/NormalEditor/Shaders/BakeNormalMap.shader"));
            if (m_csBakeFromMap == null)
                m_csBakeFromMap = AssetDatabase.LoadAssetAtPath<ComputeShader>("Assets/UTJ/NormalEditor/Shaders/GetNormalsFromTexture.compute");

            if (m_meshTarget == null ||
                m_meshTarget.vertexCount != GetComponent<MeshFilter>().sharedMesh.vertexCount ||
                (m_points != null && m_meshTarget.vertexCount != m_points.Length))
            {
                m_meshTarget = GetComponent<MeshFilter>().sharedMesh;
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

        void OnDestroy()
        {
            ReleaseComputeBuffers();
        }

        void Reset()
        {
            SetupResources();
        }

        public int OnSceneGUI()
        {
            if (m_points == null)
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
            if (et == EventType.ScrollWheel && e.control)
            {
                m_settings.brushRadius = m_settings.brushRadius + e.delta.y * 0.01f;
                e.Use();
                ret |= (int)SceneGUIState.Repaint;
            }

            if (et == EventType.KeyDown || et == EventType.KeyUp)
            {
                ret |= HandleKeyEvent(e, et, id);
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

            if(et == EventType.MouseMove)
            {
                if(Raycast(e, ref m_rayPos))
                    ret |= (int)SceneGUIState.Repaint;
                else
                    m_rayPos = Vector3.one * 1000000.0f;
            }

            if (m_numSelected > 0 && editMode == EditMode.Move)
            {
                if (et == EventType.MouseDown)
                    m_prevMove = m_settings.pivotPos;

                EditorGUI.BeginChangeCheck();
                var move = Handles.PositionHandle(m_settings.pivotPos, Quaternion.identity);
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
                if (et == EventType.MouseDown)
                    m_prevRot = Quaternion.identity;

                EditorGUI.BeginChangeCheck();
                var rot = Handles.RotationHandle(Quaternion.identity, m_settings.pivotPos);
                if (EditorGUI.EndChangeCheck())
                {
                    handled = true;
                    var diff = Quaternion.Inverse(m_prevRot) * rot;
                    m_prevRot = rot;
                    if (m_settings.rotatePivot)
                        ApplyRotatePivot(diff, m_settings.pivotPos, 1.0f);
                    else
                        ApplyRotate(diff);
                }
            }
            else if (m_numSelected > 0 && editMode == EditMode.Scale)
            {
                if (et == EventType.MouseDown)
                    m_prevScale = Vector3.one;

                EditorGUI.BeginChangeCheck();
                var scale = Handles.ScaleHandle(Vector3.one, m_settings.pivotPos,
                    m_settings.pivotRot, HandleUtility.GetHandleSize(m_settings.pivotPos));
                if (EditorGUI.EndChangeCheck())
                {
                    handled = true;
                    var diff = scale - m_prevScale;
                    m_prevScale = scale;
                    ApplyScale(diff, m_settings.pivotPos);
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
            var ray = HandleUtility.GUIPointToWorldRay(e.mousePosition);

            if (editMode == EditMode.Brush && m_settings.pickNormal)
            {
                var n = Vector3.zero;
                if (PickNormal(ray, ref n))
                {
                    m_settings.primary = ToColor(n);
                    handled = true;
                }
                m_settings.pickNormal = false;
            }
            else if (editMode == EditMode.Brush)
            {
                switch (m_settings.brushMode)
                {
                    case BrushMode.Paint:
                        if (ApplyAdditiveBrush(ray, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength, ToVector(m_settings.primary).normalized))
                            handled = true;
                        break;
                    case BrushMode.Scale:
                        if (ApplyScaleBrush(ray, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength))
                            handled = true;
                        break;
                    case BrushMode.Reset:
                        if (ApplyResetBrush(ray, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength))
                            handled = true;
                        break;
                    case BrushMode.Equalize:
                        if (ApplyEqualizeBrush(ray, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength))
                            handled = true;
                        break;
                }
                if (et == EventType.MouseUp && handled)
                    PushUndo();
            }
            else
            {
                var selectMode = m_settings.selectMode;
                float selectSign = e.control ? -1.0f : 1.0f;

                if (selectMode == SelectMode.Single)
                {
                    if (!e.shift && !e.control)
                        System.Array.Clear(m_selection, 0, m_selection.Length);

                    int sel = GetMouseVertex(e);
                    if (sel != -1)
                    {
                        m_selection[sel] = 1.0f;
                        handled = true;
                    }
                }
                else if (selectMode == SelectMode.Brush)
                {
                    if (et == EventType.MouseDown && !e.shift && !e.control)
                        System.Array.Clear(m_selection, 0, m_selection.Length);

                    //if (SelectHard(ray, m_settings.brushRadius, m_settings.brushStrength * selectSign))
                    //    handled = true;
                    if (SelectSoft(ray, m_settings.brushRadius, m_settings.brushFalloff, m_settings.brushStrength * selectSign))
                        handled = true;
                }
                else if (selectMode == SelectMode.Rect)
                {
                    if (et == EventType.MouseDown)
                    {
                        m_dragStartPoint = m_dragEndPoint = e.mousePosition;
                        handled = true;
                    }
                    else if (et == EventType.MouseDrag)
                    {
                        m_dragEndPoint = new Vector2(Mathf.Max(e.mousePosition.x, 0), Mathf.Max(e.mousePosition.y, 0));
                        handled = true;
                    }
                    else if (et == EventType.MouseUp)
                    {
                        if (!e.shift && !e.control)
                            System.Array.Clear(m_selection, 0, m_selection.Length);

                        m_dragEndPoint = new Vector2(Mathf.Max(e.mousePosition.x, 0), Mathf.Max(e.mousePosition.y, 0));
                        handled = true;

                        if (!SelectRect(m_dragStartPoint, m_dragEndPoint, selectSign))
                        {
                            Selection.activeGameObject = null;
                        }
                        m_dragStartPoint = m_dragEndPoint = Vector2.zero;
                    }
                }

                UpdateSelection();
            }

            if (handled)
            {
                if (et == EventType.MouseDown)
                {
                    GUIUtility.hotControl = id;
                }
                else if (et == EventType.MouseUp)
                {
                    if (GUIUtility.hotControl == id && e.button == 0)
                        GUIUtility.hotControl = 0;
                }
                ret |= (int)SceneGUIState.Repaint;
                e.Use();
            }
            return ret;
        }

        int HandleKeyEvent(Event e, EventType et, int id)
        {
            int ret = 0;
            bool handled = false;
            if (e.keyCode == KeyCode.A)
            {
                SelectAll();
                UpdateSelection();
                handled = true;
            }

            if (handled)
            {
                e.Use();
            }
            return ret;
        }


        void OnDrawGizmosSelected()
        {
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
            if (m_settings.editMode == EditMode.Brush ||
                (m_settings.editMode == EditMode.Select && m_settings.selectMode == SelectMode.Brush))
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
            Graphics.ExecuteCommandBuffer(m_cmdDraw);
        }

        void OnRepaint()
        {
            if (m_settings.selectMode == SelectMode.Rect)
            {
                var selectionRect = typeof(EditorStyles).GetProperty("selectionRect", BindingFlags.NonPublic | BindingFlags.Static);
                if (selectionRect != null)
                {
                    var style = (GUIStyle)selectionRect.GetValue(null, null);
                    Handles.BeginGUI();
                    style.Draw(FromToRect(m_dragStartPoint, m_dragEndPoint), GUIContent.none, false, false, false, false);
                    Handles.EndGUI();
                }
            }
        }

#endif
    }
}

