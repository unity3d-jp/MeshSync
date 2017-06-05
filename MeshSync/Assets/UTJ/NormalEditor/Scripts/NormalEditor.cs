using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using UnityEngine;
using UnityEngine.Rendering;
#if UNITY_EDITOR
using UnityEditor;
#endif

[ExecuteInEditMode]
[RequireComponent(typeof(MeshRenderer))]
public class NormalEditor : MonoBehaviour
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
        //Lasso,
    }
    public enum MirrorMode
    {
        None,
        RightToLeft,
        LeftToRight,
        FrontToBack,
        BackToFront,
        TopToBottom,
        BottomToTop,
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
    MirrorMode m_mirroMode = MirrorMode.None;
    float m_brushRadius = 0.2f;
    float m_brushPow = 0.5f;
    float m_brushStrength = 1.0f;

    // display options
    bool m_showVertices = true;
    bool m_showNormals = true;
    bool m_showTangents = false;
    float m_vertexSize = 0.01f;
    float m_normalSize = 0.10f;
    float m_tangentSize = 0.075f;
    Color m_vertexColor = new Color(0.15f, 0.15f, 0.4f, 0.75f);
    Color m_vertexColor2 = new Color(1.0f, 0.0f, 0.0f, 0.75f);
    Color m_normalColor = Color.yellow;
    Color m_tangentColor = Color.cyan;

    // internal resources
    [SerializeField] Mesh m_meshTarget;
    [SerializeField] Mesh m_meshCube;
    [SerializeField] Mesh m_meshLine;
    [SerializeField] Material m_material;

    ComputeBuffer m_cbArg;
    ComputeBuffer m_cbPoints;
    ComputeBuffer m_cbNormals;
    ComputeBuffer m_cbTangents;
    ComputeBuffer m_cbSelection;
    CommandBuffer m_cmdDraw;

    Vector3[] m_points;
    Vector3[] m_normals;
    Vector3[] m_baseNormals;
    Vector4[] m_tangents;
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
        get { return m_mirroMode; }
        set {
            if(value != m_mirroMode)
            {
                m_mirrorRelation = null;
                m_mirroMode = value;
            }
        }
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

        if (m_material == null)
        {
            m_material = new Material(AssetDatabase.LoadAssetAtPath<Shader>("Assets/UTJ/NormalEditor/Shaders/NormalVisualizer.shader"));
        }

        if (m_meshTarget == null ||
            m_meshTarget.vertexCount != GetComponent<MeshFilter>().sharedMesh.vertexCount)
        {
            m_meshTarget = GetComponent<MeshFilter>().sharedMesh;
            m_points = null;
            m_normals = null;
            m_baseNormals = null;
            m_tangents = null;
            m_triangles = null;
            m_selection = null;
            ReleaseComputeBuffers();
        }
        if (m_points == null && m_meshTarget != null)
        {
            m_points = m_meshTarget.vertices;
            m_normals = m_meshTarget.normals;
            m_tangents = m_meshTarget.tangents;
            m_triangles = m_meshTarget.triangles;
            m_selection = new float[m_points.Length];
            PushUndo();
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
        }
        if (m_cbTangents == null && m_tangents != null)
        {
            m_cbTangents = new ComputeBuffer(m_tangents.Length, 16);
            m_cbTangents.SetData(m_tangents);
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
    }

    void ReleaseComputeBuffers()
    {
        if (m_cbArg != null) { m_cbArg.Release(); m_cbArg = null; }
        if (m_cbPoints != null) { m_cbPoints.Release(); m_cbPoints = null; }
        if (m_cbNormals != null) { m_cbNormals.Release(); m_cbNormals = null; }
        if (m_cbTangents != null) { m_cbTangents.Release(); m_cbTangents = null; }
        if (m_cbSelection != null) { m_cbSelection.Release(); m_cbSelection = null; }
        if (m_cmdDraw != null) { m_cmdDraw.Release(); m_cmdDraw = null; }
    }


    void OnEnable()
    {
        SetupResources();
    }

    void OnDisable()
    {
        ReleaseComputeBuffers();
    }

    void Reset()
    {
        SetupResources();
    }

    void OnDrawGizmosSelected()
    {
        if(m_material == null || m_meshCube == null || m_meshLine == null)
        {
            Debug.LogWarning("NormalEditor: Some resources are missing.\n");
            return;
        }

        var trans = GetComponent<Transform>();
        var matrix = trans.localToWorldMatrix;

        m_material.SetMatrix("_Transform", matrix);
        m_material.SetFloat("_VertexSize", m_vertexSize);
        m_material.SetFloat("_NormalSize", m_normalSize);
        m_material.SetFloat("_TangentSize", m_tangentSize);
        m_material.SetColor("_VertexColor", m_vertexColor);
        m_material.SetColor("_VertexColor2", m_vertexColor2);
        m_material.SetColor("_NormalColor", m_normalColor);
        m_material.SetColor("_TangentColor", m_tangentColor);
        if (m_cbPoints != null) m_material.SetBuffer("_Points", m_cbPoints);
        if (m_cbNormals != null) m_material.SetBuffer("_Normals", m_cbNormals);
        if (m_cbTangents != null) m_material.SetBuffer("_Tangents", m_cbTangents);
        if (m_cbSelection != null) m_material.SetBuffer("_Selection", m_cbSelection);

        if (m_cmdDraw == null)
        {
            m_cmdDraw = new CommandBuffer();
            m_cmdDraw.name = "NormalEditor";
        }
        m_cmdDraw.Clear();
        if (m_showVertices && m_points != null)
            m_cmdDraw.DrawMeshInstancedIndirect(m_meshCube, 0, m_material, 0, m_cbArg);
        if (m_showTangents && m_tangents != null)
            m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_material, 2, m_cbArg);
        if (m_showNormals && m_normals != null)
            m_cmdDraw.DrawMeshInstancedIndirect(m_meshLine, 0, m_material, 1, m_cbArg);
        Graphics.ExecuteCommandBuffer(m_cmdDraw);
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
            bool used = false;
            if (e.button == 0)
            {
                var ray = HandleUtility.GUIPointToWorldRay(e.mousePosition);
                if (m_editMode != EditMode.Brush)
                {
                    if (!e.shift)
                        System.Array.Clear(m_selection, 0, m_selection.Length);

                    if (m_selectMode == SelectMode.Single)
                    {
                        int sel = GetMouseVertex(e);
                        if (sel != -1)
                        {
                            m_selection[sel] = 1.0f;
                            used = true;
                        }
                    }
                    else if (m_selectMode == SelectMode.Soft)
                    {
                        if(SoftSelection(ray, m_brushRadius, m_brushPow, m_brushStrength))
                        {
                            used = true;
                        }
                    }
                    else if(m_selectMode == SelectMode.Hard)
                    {
                        if (HardSelection(ray, m_brushRadius, m_brushStrength))
                        {
                            used = true;
                        }
                    }

                    float st = 0.0f;
                    m_numSelected = 0;
                    m_selectionPos = Vector3.zero;
                    m_selectionNormal = Vector3.zero;
                    int numPoints = m_points.Length;
                    for (int i = 0; i < numPoints; ++i)
                    {
                        float s = m_selection[i];
                        if (s > 0.0f)
                        {
                            m_selectionPos += m_points[i] * s;
                            m_selectionNormal += m_normals[i] * s;
                            ++m_numSelected;
                            st += s;
                        }
                    }
                    if (m_numSelected > 0)
                    {
                        m_selectionPos /= st;
                        m_selectionNormal /= st;
                        m_selectionNormal = m_selectionNormal.normalized;
                        m_selectionRot = Quaternion.LookRotation(m_selectionNormal);
                        m_pivotPos = m_selectionPos;
                        m_pivotRot = m_selectionRot;
                    }

                    m_cbSelection.SetData(m_selection);
                }
                else if (m_editMode == EditMode.Brush)
                {
                    switch (m_brushMode)
                    {
                        case BrushMode.Equalize:
                            if (ApplyEqualizeRaycast(ray, m_brushRadius, m_brushPow, m_brushStrength))
                                used = true;
                            break;
                    }
                    if (type == EventType.MouseUp && used)
                        PushUndo();
                }
            }

            if (used)
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
    }



    int GetMouseVertex(Event e, bool allowBackface = false)
    {
        //if (Tools.current != Tool.None)
        //{
        //    Debug.Log(Tools.current);
        //    return -1;
        //}

        Ray mouseRay = HandleUtility.GUIPointToWorldRay(e.mousePosition);
        float minDistance = float.MaxValue;
        int found = -1;
        Quaternion rotation = GetComponent<Transform>().rotation;
        for (int i = 0; i < m_points.Length; i++)
        {
            Vector3 dir = m_points[i] - mouseRay.origin;
            float sqrDistance = Vector3.Cross(dir, mouseRay.direction).sqrMagnitude;
            bool forwardFacing = Vector3.Dot(rotation * m_normals[i], Camera.current.transform.forward) <= 0;
            if ((forwardFacing || allowBackface) && sqrDistance < minDistance && sqrDistance < 0.05f * 0.05f)
            {
                minDistance = sqrDistance;
                found = i;
            }
        }
        return found;
    }


    public void SetClipboard(Vector3 v)
    {
        m_clipboard = v;
    }
    public void ApplyPaste()
    {
        if (m_numSelected > 0)
        {
            for (int i = 0; i < m_points.Length; i++)
            {
                float s = m_selection[i];
                if (s > 0.0f)
                {
                    m_normals[i] = Vector3.Lerp(m_normals[i], m_clipboard, s).normalized;
                }
            }
            ApplyNewNormals();
        }
    }

    public void ApplyMove(Vector3 move)
    {
        for (int i = 0; i < m_selection.Length; ++i)
        {
            float s = m_selection[i];
            if (s > 0.0f)
            {
                m_normals[i] = (m_normals[i] + move * s).normalized;
            }
        }
        ApplyNewNormals();
    }

    public void ApplyRotation(Quaternion rot, Vector3 pivot)
    {
        for (int i = 0; i < m_selection.Length; ++i)
        {
            float s = m_selection[i];
            if (s > 0.0f)
            {
                m_normals[i] = Vector3.Lerp(m_normals[i], rot * m_normals[i], s).normalized;
            }
        }
        ApplyNewNormals();
    }

    public void ApplyScale(Vector3 size, Vector3 pivot, Quaternion rot)
    {
        for (int i = 0; i < m_selection.Length; ++i)
        {
            float s = m_selection[i];
            if (s > 0.0f)
            {
                var dir = (m_points[i] - pivot).normalized;
                dir.x *= size.x;
                dir.y *= size.y;
                dir.z *= size.z;
                m_normals[i] = (m_normals[i] + dir * s).normalized;
            }
        }
        ApplyNewNormals();
    }

    public void ApplyEqualize(float strength)
    {

    }

    public bool ApplyEqualizeRaycast(Ray ray, float radius, float pow, float strength)
    {
        Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
        if (neEqualizeRaycast(ray.origin, ray.direction,
            m_points, m_triangles, m_points.Length, m_triangles.Length / 3, radius, pow, strength, m_normals, ref trans) > 0)
        {
            ApplyNewNormals();
            return true;
        }
        return false;
    }

    public void PushUndo()
    {
        Undo.RecordObject(this, "NormalEditor");
        m_history.count++;
        m_history.normals = (Vector3[])m_normals.Clone();
    }

    public void OnUndoRedo()
    {
        m_normals = m_history.normals;
        ApplyNewNormals();
    }

    public void ApplyNewNormals(bool upload = true)
    {
        if (m_cbNormals != null)
            m_cbNormals.SetData(m_normals);

        if (m_meshTarget != null)
        {
            m_meshTarget.normals = m_normals;
            if (upload)
                m_meshTarget.UploadMeshData(false);
        }
    }


    public bool Raycast(Ray ray, ref int ti, ref float distance)
    {
        Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
        bool ret = neRaycast(ray.origin, ray.direction,
            m_points, m_triangles, m_triangles.Length / 3, ref ti, ref distance, ref trans) > 0;
        return ret;
    }

    public bool SoftSelection(Ray ray, float radius, float pow, float strength)
    {
        Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
        bool ret = neSoftSelection(ray.origin, ray.direction,
            m_points, m_triangles, m_points.Length, m_triangles.Length/3, radius, pow, strength, m_selection, ref trans) > 0;
        return ret;
    }

    public bool HardSelection(Ray ray, float radius, float strength)
    {
        Matrix4x4 trans = GetComponent<Transform>().localToWorldMatrix;
        bool ret = neHardSelection(ray.origin, ray.direction,
            m_points, m_triangles, m_points.Length, m_triangles.Length / 3, radius, strength, m_selection, ref trans) > 0;
        return ret;
    }

    public void ApplyMirroring(Plane plane)
    {
        //if (m_mirrorRelation == null || m_mirrorRelation.Length != m_normals.Length)
        //{
        //    m_mirrorRelation = new int[m_normals.Length];
        //    neBuildMirroringRelation(m_points, m_points.Length, plane.normal, plane.distance, 0.001f, m_mirrorRelation);
        //}
        //neApplyMirroring(m_mirrorRelation, m_normals.Length, m_normals);
    }


    [DllImport("MeshSyncServer")] static extern int neRaycast(
        Vector3 pos, Vector3 dir, Vector3[] vertices, int[] indices, int num_triangles,
        ref int tindex, ref float distance, ref Matrix4x4 trans);

    [DllImport("MeshSyncServer")] static extern int neSoftSelection(
        Vector3 pos, Vector3 dir, Vector3[] vertices, int[] indices, int num_vertices, int num_triangles,
        float radius, float strength, float pow, float[] seletion, ref Matrix4x4 trans);
    
    [DllImport("MeshSyncServer")] static extern int neHardSelection(
        Vector3 pos, Vector3 dir, Vector3[] vertices, int[] indices, int num_vertices, int num_triangles,
        float radius, float strength, float[] seletion, ref Matrix4x4 trans);

    [DllImport("MeshSyncServer")] static extern int neEqualize(
        int num_vertices, int num_triangles,
        float radius, float strength, float pow, Vector3[] normals, ref Matrix4x4 trans);

    [DllImport("MeshSyncServer")] static extern int neEqualizeRaycast(
        Vector3 pos, Vector3 dir, Vector3[] vertices, int[] indices, int num_vertices, int num_triangles,
        float radius, float strength, float pow, Vector3[] normals, ref Matrix4x4 trans);

    [DllImport("MeshSyncServer")] static extern int neBuildMirroringRelation(
        Vector3[] vertices, int num_vertices, Vector3 plane_normal, float plane_distance, float epsilon, int[] relation);

    [DllImport("MeshSyncServer")] static extern void neApplyMirroring(
        int[] relation, int num_vertices, Vector3[] normals);

#endif
}