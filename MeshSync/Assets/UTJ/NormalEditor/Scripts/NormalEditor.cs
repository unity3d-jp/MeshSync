using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif

[ExecuteInEditMode]
[RequireComponent(typeof(MeshRenderer))]
public partial class NormalEditor : MonoBehaviour
{
#if UNITY_EDITOR
    public enum EditMode
    {
        Select,
        Brush,
        Move,
        Rotate,
        Scale,
    }
    public enum BrushMode
    {
        Equalize,
        Add,
        Scale,
    }
    public enum SelectMode
    {
        Single,
        Hard,
        Soft,
        Rect,
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

    [Serializable]
    class History
    {
        public int count = 0;
        public Vector3[] normals;
    }


    // edit options
    EditMode m_editMode = EditMode.Select;
    BrushMode m_brushMode = BrushMode.Equalize;
    SelectMode m_selectMode = SelectMode.Soft;
    MirrorMode m_mirrorMode = MirrorMode.None;
    bool m_selectFrontSideOnly = true;
    float m_brushRadius = 0.2f;
    float m_brushPow = 0.5f;
    float m_brushStrength = 1.0f;

    // display options
    bool m_showVertices = true;
    bool m_showNormals = true;
    bool m_showTangents = false;
    bool m_showBinormals = false;
    bool m_showTangentSpaceNormals = false;
    float m_vertexSize = 0.0075f;
    float m_normalSize = 0.10f;
    float m_tangentSize = 0.075f;
    float m_binormalSize = 0.06f;
    Color m_vertexColor = new Color(0.15f, 0.15f, 0.4f, 0.75f);
    Color m_vertexColor2 = new Color(1.0f, 0.0f, 0.0f, 0.75f);
    Color m_normalColor = Color.yellow;
    Color m_tangentColor = Color.cyan;
    Color m_binormalColor = Color.green;

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

    Vector3[] m_points;
    Vector3[] m_normals;
    Vector3[] m_baseNormals;
    Vector4[] m_tangents;
    Vector4[] m_baseTangents;
    int[] m_triangles;
    int[] m_mirrorRelation;
    float[] m_selection;

    int m_numSelected = 0;
    Vector3 m_selectionPos;
    Vector3 m_selectionNormal;
    Quaternion m_selectionRot;
    Vector3 m_clipboard = Vector3.up;
    Vector3 m_rayPos;
    Vector3 m_pivotPos;
    Quaternion m_pivotRot;
    Vector2 m_dragStartPoint;
    Vector2 m_dragEndPoint;

    [SerializeField] History m_history = new History();


    public Mesh mesh { get { return m_meshTarget; } }


    public EditMode editMode
    {
        get { return m_editMode; }
        set { m_editMode = value; }
    }
    public BrushMode brushMode
    {
        get { return m_brushMode; }
        set { m_brushMode = value; }
    }
    public SelectMode selectMode
    {
        get { return m_selectMode; }
        set { m_selectMode = value; }
    }

    public MirrorMode mirroMode
    {
        get { return m_mirrorMode; }
        set {
            if(value != m_mirrorMode)
            {
                m_mirrorRelation = null;
                m_mirrorMode = value;
                ApplyMirroring();
                PushUndo();
            }
        }
    }
    public bool selectFrontSideOnly
    {
        get { return m_selectFrontSideOnly; }
        set { m_selectFrontSideOnly = value; }
    }
    public float brushRadius
    {
        get { return m_brushRadius; }
        set { m_brushRadius = value; }
    }
    public float brushPow
    {
        get { return m_brushPow; }
        set { m_brushPow = value; }
    }
    public float brushStrength
    {
        get { return m_brushStrength; }
        set { m_brushStrength = value; }
    }

    public bool showVertices
    {
        get { return m_showVertices; }
        set { m_showVertices = value; }
    }
    public bool showNormals
    {
        get { return m_showNormals; }
        set { m_showNormals = value; }
    }
    public bool showTangents
    {
        get { return m_showTangents; }
        set { m_showTangents = value; }
    }
    public bool showBinormals
    {
        get { return m_showBinormals; }
        set { m_showBinormals = value; }
    }
    public bool showTangentSpaceNormals
    {
        get { return m_showTangentSpaceNormals; }
        set { m_showTangentSpaceNormals = value; }
    }

    public float vertexSize
    {
        get { return m_vertexSize; }
        set { m_vertexSize = value; }
    }
    public float normalSize
    {
        get { return m_normalSize; }
        set { m_normalSize = value; }
    }
    public float tangentSize
    {
        get { return m_tangentSize; }
        set { m_tangentSize = value; }
    }
    public float binormalSize
    {
        get { return m_binormalSize; }
        set { m_binormalSize = value; }
    }

    public Color vertexColor
    {
        get { return m_vertexColor; }
        set { m_vertexColor = value; }
    }
    public Color vertexColor2
    {
        get { return m_vertexColor2; }
        set { m_vertexColor2 = value; }
    }
    public Color normalColor
    {
        get { return m_normalColor; }
        set { m_normalColor = value; }
    }
    public Color tangentColor
    {
        get { return m_tangentColor; }
        set { m_tangentColor = value; }
    }
    public Color binormalColor
    {
        get { return m_binormalColor; }
        set { m_binormalColor = value; }
    }



    void SetupResources()
    {
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
            m_matVisualize = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/NormalEditor/Shaders/NormalVisualizer.shader"));
        if (m_matBake == null)
            m_matBake = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/NormalEditor/Shaders/BakeNormalMap.shader"));
        if (m_csBakeFromMap == null)
            m_csBakeFromMap = AssetDatabase.LoadAssetAtPath<ComputeShader>("Assets/UTJ/NormalEditor/Shaders/BakeFromMap.compute");

        if (m_meshTarget == null ||
            m_meshTarget.vertexCount != GetComponent<MeshFilter>().sharedMesh.vertexCount)
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
        if (m_points == null && m_meshTarget != null)
        {
            m_points = m_meshTarget.vertices;

            m_normals = m_meshTarget.normals;
            if(m_normals.Length == 0)
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
            if(m_tangents.Length == 0)
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

        if (m_cbPoints == null && m_points != null)
        {
            m_cbPoints = new ComputeBuffer(m_points.Length, 12);
            m_cbPoints.SetData(m_points);
        }
        if (m_cbNormals == null && m_normals != null)
        {
            m_cbNormals = new ComputeBuffer(m_normals.Length, 12);
            m_cbNormals.SetData(m_normals);
            m_cbBaseNormals = new ComputeBuffer(m_baseNormals.Length, 12);
            m_cbBaseNormals.SetData(m_baseNormals);
        }
        if (m_cbTangents == null && m_tangents != null)
        {
            m_cbTangents = new ComputeBuffer(m_tangents.Length, 16);
            m_cbTangents.SetData(m_tangents);
            m_cbBaseTangents = new ComputeBuffer(m_baseTangents.Length, 16);
            m_cbBaseTangents.SetData(m_baseTangents);
        }
        if (m_cbSelection == null && m_selection != null)
        {
            m_cbSelection = new ComputeBuffer(m_selection.Length, 4);
            m_cbSelection.SetData(m_selection);
        }

        if (m_cbArg == null && m_points != null)
        {
            m_cbArg = new ComputeBuffer(1, 5 * sizeof(uint), ComputeBufferType.IndirectArguments);
            m_cbArg.SetData(new uint[5] { m_meshCube.GetIndexCount(0), (uint)m_points.Length, 0, 0, 0 });
        }

        UpdateNormals();
        PushUndo();
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

    void Reset()
    {
        SetupResources();
    }

    public void OnSceneGUI()
    {
        {
            if(m_numSelected > 0)
            {
                int numPoints = m_points.Length;

                if (m_editMode == EditMode.Move)
                {
                    EditorGUI.BeginChangeCheck();
                    var pos = Handles.PositionHandle(m_pivotPos, m_selectionRot);
                    if (EditorGUI.EndChangeCheck())
                    {
                        var move = pos - m_pivotPos;
                        ApplyMove(move);
                        PushUndo();
                    }

                }
                else if (m_editMode == EditMode.Rotate)
                {
                    EditorGUI.BeginChangeCheck();
                    var rot = Handles.RotationHandle(m_selectionRot, m_pivotPos);
                    if (EditorGUI.EndChangeCheck())
                    {
                        var diff = Quaternion.Inverse(m_selectionRot) * rot;
                        ApplyRotation(diff, m_pivotPos);
                        PushUndo();
                    }
                }
                else if (m_editMode == EditMode.Scale)
                {
                    EditorGUI.BeginChangeCheck();
                    var scale = Handles.ScaleHandle(Vector3.one, m_pivotPos, m_selectionRot, HandleUtility.GetHandleSize(m_pivotPos));
                    if (EditorGUI.EndChangeCheck())
                    {
                        var size = scale - Vector3.one;
                        ApplyScale(size, m_pivotPos, m_pivotRot);
                        PushUndo();
                    }
                }
            }
        }



        int id = GUIUtility.GetControlID(FocusType.Passive);

        Event e = Event.current;
        var type = e.GetTypeForControl(id);
        if (type == EventType.MouseDown || type == EventType.MouseDrag || type == EventType.MouseUp)
        {
            bool handled = false;
            if (e.button == 0)
            {
                var ray = HandleUtility.GUIPointToWorldRay(e.mousePosition);
                if (m_editMode != EditMode.Brush)
                {
                    if (!e.shift && type == EventType.MouseUp)
                        System.Array.Clear(m_selection, 0, m_selection.Length);

                    if (m_selectMode == SelectMode.Single)
                    {
                        int sel = GetMouseVertex(e);
                        if (sel != -1)
                        {
                            m_selection[sel] = 1.0f;
                            handled = true;
                        }
                    }
                    else if (m_selectMode == SelectMode.Hard)
                    {
                        if (SelectHard(ray, m_brushRadius, m_brushStrength))
                        {
                            handled = true;
                        }
                    }
                    else if (m_selectMode == SelectMode.Soft)
                    {
                        if (SelectSoft(ray, m_brushRadius, m_brushPow, m_brushStrength))
                        {
                            handled = true;
                        }
                    }
                    else if (m_selectMode == SelectMode.Rect)
                    {
                        if (type == EventType.MouseDown)
                        {
                            m_dragStartPoint = m_dragEndPoint = e.mousePosition;
                            handled = true;
                        }
                        else if (type == EventType.MouseDrag || type == EventType.MouseUp)
                        {
                            m_dragEndPoint = new Vector2(Mathf.Max(e.mousePosition.x, 0), Mathf.Max(e.mousePosition.y, 0));
                            handled = true;
                            if (type == EventType.MouseUp)
                            {
                                if (!SelectRect(m_dragStartPoint, m_dragEndPoint))
                                {
                                    Selection.activeGameObject = null;
                                }
                                m_dragStartPoint = m_dragEndPoint = Vector2.zero;
                            }
                        }
                    }

                    UpdateSelection();
                }
                else if (m_editMode == EditMode.Brush)
                {
                    switch (m_brushMode)
                    {
                        case BrushMode.Equalize:
                            if (ApplyEqualizeBrush(ray, m_brushRadius, m_brushPow, m_brushStrength))
                                handled = true;
                            break;
                    }
                    if (type == EventType.MouseUp && handled)
                        PushUndo();
                }
            }

            if (handled)
            {
                if (type == EventType.MouseDown)
                {
                    GUIUtility.hotControl = id;
                }
                else if (type == EventType.MouseUp)
                {
                    if (GUIUtility.hotControl == id && e.button == 0)
                        GUIUtility.hotControl = 0;
                }
                e.Use();
            }
        }
        else if(type == EventType.KeyDown || type == EventType.KeyUp)
        {

        }


        if (Event.current.type == EventType.Repaint)
            OnRepaint();
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
        m_matVisualize.SetFloat("_VertexSize", m_vertexSize);
        m_matVisualize.SetFloat("_NormalSize", m_normalSize);
        m_matVisualize.SetFloat("_TangentSize", m_tangentSize);
        m_matVisualize.SetFloat("_BinormalSize", m_binormalSize);
        m_matVisualize.SetColor("_VertexColor", m_vertexColor);
        m_matVisualize.SetColor("_VertexColor2", m_vertexColor2);
        m_matVisualize.SetColor("_NormalColor", m_normalColor);
        m_matVisualize.SetColor("_TangentColor", m_tangentColor);
        m_matVisualize.SetColor("_BinormalColor", m_binormalColor);
        if (m_cbPoints != null) m_matVisualize.SetBuffer("_Points", m_cbPoints);
        if (m_cbNormals != null) m_matVisualize.SetBuffer("_Normals", m_cbNormals);
        if (m_cbTangents != null) m_matVisualize.SetBuffer("_Tangents", m_cbTangents);
        if (m_cbSelection != null) m_matVisualize.SetBuffer("_Selection", m_cbSelection);
        if (m_showTangentSpaceNormals)
        {
            if (m_cbBaseNormals != null) m_matBake.SetBuffer("_BaseNormals", m_cbBaseNormals);
            if (m_cbBaseTangents != null) m_matBake.SetBuffer("_BaseTangents", m_cbBaseTangents);
        }

        if (m_cmdDraw == null)
        {
            m_cmdDraw = new CommandBuffer();
            m_cmdDraw.name = "NormalEditor";
        }
        m_cmdDraw.Clear();
        if(m_showTangentSpaceNormals)
            for (int si = 0; si < m_meshTarget.subMeshCount; ++si)
                m_cmdDraw.DrawMesh(m_meshTarget, matrix, m_matBake, si, 0);
        if (m_showVertices && m_points != null)
            m_cmdDraw.DrawMeshInstancedIndirect(m_meshCube, 0, m_matVisualize, 0, m_cbArg);
        if (m_showBinormals && m_tangents != null)
            m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_matVisualize, 3, m_cbArg);
        if (m_showTangents && m_tangents != null)
            m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_matVisualize, 2, m_cbArg);
        if (m_showNormals && m_normals != null)
            m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_matVisualize, 1, m_cbArg);
        Graphics.ExecuteCommandBuffer(m_cmdDraw);
    }

    void OnRepaint()
    {
        if (m_selectMode == SelectMode.Rect)
        {
            var selectionRect = typeof(EditorStyles).GetProperty("selectionRect", BindingFlags.NonPublic | BindingFlags.Static);
            var style = (GUIStyle)selectionRect.GetValue(null, null);
            Handles.BeginGUI();
            style.Draw(FromToRect(m_dragStartPoint, m_dragEndPoint), GUIContent.none, false, false, false, false);
            Handles.EndGUI();
        }
    }

#endif
}