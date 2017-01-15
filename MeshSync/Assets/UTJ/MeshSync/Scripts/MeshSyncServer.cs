using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Reflection;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif


[ExecuteInEditMode]
public class MeshSyncServer : MonoBehaviour
{
    #region internal
    public enum EventType
    {
        Unknown,
        Get,
        Delete,
        Xform,
        Mesh,
    }

    public struct GetFlags
    {
        public int flags;
        public bool get_xforms { get { return (flags & 0x1) != 0; } }
        public bool get_meshes { get { return (flags & 0x2) != 0; } }
        public bool mesh_get_points { get { return (flags & 0x4) != 0; } }
        public bool mesh_get_normals { get { return (flags & 0x8) != 0; } }
        public bool mesh_get_tangents { get { return (flags & 0x10) != 0; } }
        public bool mesh_get_uv { get { return (flags & 0x20) != 0; } }
        public bool mesh_get_indices { get { return (flags & 0x40) != 0; } }
        public bool mesh_get_bones { get { return (flags & 0x80) != 0; } }
        public bool mesh_swap_handedness { get { return (flags & 0x100) != 0; } }
        public bool mesh_swap_faces { get { return (flags & 0x200) != 0; } }
        public bool mesh_apply_transform { get { return (flags & 0x400) != 0; } }
        public bool mesh_bake_skin { get { return (flags & 0x800) != 0; } }
    }

    public struct GetData
    {
        public GetFlags flags;
    }

    public struct DeleteData
    {
        public IntPtr obj_path;
    }

    public struct XformData
    {
        public IntPtr     obj_path;
        public Vector3    position;
        public Quaternion rotation;
        public Vector3    scale;
        public Matrix4x4  transform;
    }


    public struct MeshData
    {
        public IntPtr cpp;
        public IntPtr obj_path;
        public IntPtr points;
        public IntPtr normals;
        public IntPtr tangents;
        public IntPtr uv;
        public IntPtr indices;
        public int    num_points;
        public int    num_indices;
        public int    num_submeshes;
        public Matrix4x4 transform;
    };
    public struct SubmeshData
    {
        public IntPtr points;
        public IntPtr normals;
        public IntPtr tangents;
        public IntPtr uv;
        public IntPtr indices;
        public int    num_points;
        public int    num_indices;
    };

    public struct MeshRefineFlags
    {
        public int flags;
        public bool split
        {
            get { return (flags & 0x1) != 0; }
            set { SwitchBit(ref flags, value, 0x1); }
        }
        public bool genNormals
        {
            get { return (flags & 0x2) != 0; }
            set { SwitchBit(ref flags, value, 0x2); }
        }
        public bool genTangents
        {
            get { return (flags & 0x4) != 0; }
            set { SwitchBit(ref flags, value, 0x4); }
        }
        public bool swapHandedness
        {
            get { return (flags & 0x8) != 0; }
            set { SwitchBit(ref flags, value, 0x8); }
        }
        public bool swapFaces
{
            get { return (flags & 0x10) != 0; }
            set { SwitchBit(ref flags, value, 0x10); }
        }
    }

    public struct MeshRefineSettings
    {
        public MeshRefineFlags flags;
        public float scale;
        public int split_unit;

        public static MeshRefineSettings default_value
        {
            get
            {
                return new MeshRefineSettings
                {
                    scale = 1.0f,
                    split_unit = 65000,
                };
            }
        }
    }

    public struct ServerSettings
    {
        public int max_queue;
        public int max_threads;
        public ushort port;
        public MeshRefineSettings mrs;

        public static ServerSettings default_value
        {
            get
            {
                return new ServerSettings
                {
                    max_queue = 100,
                    max_threads = 4,
                    port = 8080,
                    mrs = MeshRefineSettings.default_value,
                };
            }
        }
    }

    [DllImport("MeshSyncServer")] public static extern IntPtr msServerStart(ref ServerSettings settings);
    [DllImport("MeshSyncServer")] public static extern void msServerStop(IntPtr sv);

    public delegate void msEventHandler(EventType type, IntPtr data);
    [DllImport("MeshSyncServer")] public static extern void msServerProcessEvents(IntPtr sv, msEventHandler handler);

    [DllImport("MeshSyncServer")] public static extern void msServerBeginServe(IntPtr sv);
    [DllImport("MeshSyncServer")] public static extern void msServerEndServe(IntPtr sv);
    [DllImport("MeshSyncServer")] public static extern void msServerAddServeData(IntPtr sv, EventType et, ref XformData data);
    [DllImport("MeshSyncServer")] public static extern void msServerAddServeData(IntPtr sv, EventType et, ref MeshData data);

    [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref GetData dst, IntPtr src);
    [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref DeleteData dst, IntPtr src);
    [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref XformData dst, IntPtr src);
    [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref MeshData dst, ref MeshData src);
    [DllImport("MeshSyncServer")] public static extern void msCopyData(EventType et, ref MeshData dst, IntPtr src);

    [DllImport("MeshSyncServer")] public static extern IntPtr msCreateString(string str);
    [DllImport("MeshSyncServer")] public static extern void msDeleteString(IntPtr str);

    [DllImport("MeshSyncServer")] public static extern void msGetSubmeshData(ref SubmeshData dst, ref MeshData v, int i);
    [DllImport("MeshSyncServer")] public static extern void msCopySubmeshData(ref SubmeshData dst, ref SubmeshData src);


    static void SwitchBit(ref int flags, bool f, int bit)
    {

        if (f) { flags |= bit; }
        else { flags &= ~bit; }
    }

    public static IntPtr RawPtr(Array v)
    {
        return v == null ? IntPtr.Zero : Marshal.UnsafeAddrOfPinnedArrayElement(v, 0);
    }
    public static string S(IntPtr cstring)
    {
        return cstring == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(cstring);
    }
    #endregion


    #region fields
    [SerializeField] int m_port = 8080;
    [SerializeField] float m_scale = 0.001f;
    [SerializeField] bool m_genNormals = true;
    [SerializeField] bool m_genTangents = false;
    [SerializeField] bool m_swapHandedness= true;
    [SerializeField] bool m_swapFaces = false;
    IntPtr m_server;
    msEventHandler m_handler;

    Vector3[] m_points;
    Vector3[] m_normals;
    Vector4[] m_tangents;
    Vector2[] m_uv;
    int[] m_indices;
    #endregion



    #region impl
    void StartServer()
    {
        StopServer();

        var settings = ServerSettings.default_value;
        settings.mrs.flags.split = true;
        settings.mrs.flags.genNormals = m_genNormals;
        settings.mrs.flags.genTangents = m_genTangents;
        settings.mrs.flags.swapHandedness = m_swapHandedness;
        settings.mrs.flags.swapFaces = m_swapFaces;
        settings.mrs.scale = m_scale;
        settings.port = (ushort)m_port;
        m_server = msServerStart(ref settings);
        m_handler = OnServerEvent;
    }

    void StopServer()
    {
        msServerStop(m_server);
        m_server = IntPtr.Zero;
    }

    void PollServerEvents()
    {
        msServerProcessEvents(m_server, m_handler);
    }

    void OnServerEvent(EventType type, IntPtr data)
    {
        switch (type)
        {
        case EventType.Get:
            OnRecvGet(data);
            break;
        case EventType.Delete:
            OnRecvDelete(data);
            break;
        case EventType.Xform:
            OnRecvXform(data);
            break;
        case EventType.Mesh:
            OnRecvMesh(data);
            break;
        }
    }

    void OnRecvGet(IntPtr data)
    {
        var edata = default(GetData);
        msCopyData(EventType.Get, ref edata, data);

        msServerBeginServe(m_server);
        foreach (var mr in FindObjectsOfType<MeshRenderer>())
        {
            ServeData(mr, edata.flags);
        }
        foreach (var smr in FindObjectsOfType<SkinnedMeshRenderer>())
        {
            ServeData(smr, edata.flags);
        }
        msServerEndServe(m_server);

        Debug.Log("MeshSyncServer: Get");
    }

    void OnRecvDelete(IntPtr data)
    {
        var edata = default(DeleteData);
        msCopyData(EventType.Delete, ref edata, data);
        var path = S(edata.obj_path);
        var target = FindObjectByPath(null, path);
        if(target != null)
        {
            DestroyImmediate(target.gameObject);
        }

        Debug.Log("MeshSyncServer: Delete " + path);
    }

    void OnRecvXform(IntPtr data)
    {
        var edata = default(XformData);
        msCopyData(EventType.Xform, ref edata, data);
        var path = S(edata.obj_path);
        var target = FindObjectByPath(null, path);
        if (target != null)
        {
            target.localPosition = edata.position;
            target.localRotation = edata.rotation;
            target.localScale = edata.scale;
        }

        Debug.Log("MeshSyncServer: Xform " + path);
    }

    void OnRecvMesh(IntPtr data)
    {
        var edata = default(MeshData);
        msCopyData(EventType.Mesh, ref edata, data);
        var path = S(edata.obj_path);

        Transform target = FindObjectByPath(null, path, true);
        if (target == null) { return; }

        if (edata.num_submeshes == 0)
        {
            var mdata = default(MeshData);
            if (edata.points != IntPtr.Zero)
            {
                m_points = new Vector3[edata.num_points];
                mdata.points = RawPtr(m_points);
            }
            if (edata.normals != IntPtr.Zero)
            {
                m_normals = new Vector3[edata.num_points];
                mdata.normals = RawPtr(m_normals);
            }
            if (edata.tangents != IntPtr.Zero)
            {
                m_tangents = new Vector4[edata.num_points];
                mdata.tangents = RawPtr(m_tangents);
            }
            if (edata.uv != IntPtr.Zero)
            {
                m_uv = new Vector2[edata.num_points];
                mdata.uv = RawPtr(m_uv);
            }
            if (edata.indices != IntPtr.Zero)
            {
                m_indices = new int[edata.num_indices];
                mdata.indices = RawPtr(m_indices);
            }
            msCopyData(EventType.Mesh, ref mdata, ref edata);

            var mesh = GetOrAddMeshComponents(target.gameObject);
            if(!mesh.isReadable) { return; }
            mesh.Clear();
            if (mdata.num_points > 0 && mdata.num_indices > 0)
            {
                if (mdata.points != IntPtr.Zero) { mesh.vertices = m_points; }
                if (mdata.normals != IntPtr.Zero) { mesh.normals = m_normals; }
                if (mdata.tangents != IntPtr.Zero) { mesh.tangents = m_tangents; }
                if (mdata.uv != IntPtr.Zero) { mesh.uv = m_uv; }
                mesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                mesh.UploadMeshData(false);
            }
        }
        else
        {
            for (int i = 0; i < edata.num_submeshes; ++i)
            {
                var smd = default(SubmeshData);
                msGetSubmeshData(ref smd, ref edata, i);

                var mdata = default(SubmeshData);
                if (smd.points != IntPtr.Zero)
                {
                    m_points = new Vector3[smd.num_points];
                    mdata.points = RawPtr(m_points);
                }
                if (smd.normals != IntPtr.Zero)
                {
                    m_normals = new Vector3[smd.num_points];
                    mdata.normals = RawPtr(m_normals);
                }
                if (smd.tangents != IntPtr.Zero)
                {
                    m_tangents = new Vector4[smd.num_points];
                    mdata.tangents = RawPtr(m_tangents);
                }
                if (smd.uv != IntPtr.Zero)
                {
                    m_uv = new Vector2[smd.num_points];
                    mdata.uv = RawPtr(m_uv);
                }
                if (smd.indices != IntPtr.Zero)
                {
                    m_indices = new int[smd.num_indices];
                    mdata.indices = RawPtr(m_indices);
                }
                msCopySubmeshData(ref mdata, ref smd);

                var t = target;
                if (i > 0)
                {
                    t = FindObjectByPath(null, path + "/Submesh[" + i + "]", true);
                }

                var mesh = GetOrAddMeshComponents(t.gameObject);
                if (!mesh.isReadable) { return; }
                mesh.Clear();
                if (mdata.num_points > 0 && mdata.num_indices > 0)
                {
                    if (mdata.points != IntPtr.Zero) { mesh.vertices = m_points; }
                    if (mdata.normals != IntPtr.Zero) { mesh.normals = m_normals; }
                    if (mdata.tangents != IntPtr.Zero) { mesh.tangents = m_tangents; }
                    if (mdata.uv != IntPtr.Zero) { mesh.uv = m_uv; }
                    mesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
                    mesh.UploadMeshData(false);
                }
            }
        }

        int num_submeshes = Math.Max(1, edata.num_submeshes);
        for (int i = num_submeshes; ; ++i)
        {
            var t = FindObjectByPath(null, path + "/Submesh[" + i + "]");
            if (t == null) { break; }
            DestroyImmediate(t.gameObject);
        }

        ForceRepaint();
        Debug.Log("MeshSyncServer: Mesh " + path);
    }


    Transform FindObjectByPath(Transform parent, string path, bool createIfNotExist = false)
    {
        var names = path.Split('/');
        Transform t = null;
        foreach (var name in names)
        {
            if (name.Length == 0) { continue; }
            t = FindObjectByName(t, name, createIfNotExist);
            if(t == null) { break; }
        }
        return t;
    }

    Transform FindObjectByName(Transform parent, string name, bool createIfNotExist = false)
    {
        Transform ret = null;
        if (parent == null)
        {
            var roots = UnityEngine.SceneManagement.SceneManager.GetActiveScene().GetRootGameObjects();
            foreach(var go in roots)
            {
                if(go.name == name)
                {
                    ret = go.GetComponent<Transform>();
                    break;
                }
            }
        }
        else
        {
            ret = parent.FindChild(name);
        }

        if (createIfNotExist && ret == null)
        {
            var go = new GameObject();
            go.name = name;
            ret = go.GetComponent<Transform>();
            if(parent != null)
            {
                ret.SetParent(parent, false);
            }
        }
        return ret;
    }

#if UNITY_EDITOR
    static MethodInfo s_GetBuiltinExtraResourcesMethod;
    public static Material GetDefaultMaterial()
    {
        if (s_GetBuiltinExtraResourcesMethod == null)
        {
            BindingFlags bfs = BindingFlags.NonPublic | BindingFlags.Static;
            s_GetBuiltinExtraResourcesMethod = typeof(EditorGUIUtility).GetMethod("GetBuiltinExtraResource", bfs);
        }
        return (Material)s_GetBuiltinExtraResourcesMethod.Invoke(null, new object[] { typeof(Material), "Default-Material.mat" });
    }
#endif

    Mesh GetOrAddMeshComponents(GameObject go)
    {
        var smr = go.GetComponent<SkinnedMeshRenderer>();
        if(smr != null)
        {
            return smr.sharedMesh;
        }

        var mfilter = go.GetComponent<MeshFilter>();
        if(mfilter == null)
        {
            mfilter = go.AddComponent<MeshFilter>();
            var mesh = new Mesh();
            mfilter.sharedMesh = mesh;
            mesh.MarkDynamic();

            var renderer = go.AddComponent<MeshRenderer>();
            if(go.name.StartsWith("Submesh"))
            {
                var parent = go.GetComponent<Transform>().parent.GetComponent<Renderer>();
                renderer.sharedMaterials = parent.sharedMaterials;
            }
            else
            {
#if UNITY_EDITOR
                Material[] materials = new Material[] { UnityEngine.Object.Instantiate(GetDefaultMaterial()) };
                materials[0].name = "Material_0";
                renderer.sharedMaterials = materials;
#endif
            }
        }
        return mfilter.sharedMesh;
    }

    void ForceRepaint()
    {
#if UNITY_EDITOR
        UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
#endif
    }


    void ServeData(MeshRenderer mr, GetFlags flags)
    {
        var mesh = mr.GetComponent<MeshFilter>().sharedMesh;
        if (mesh == null) return;
        if(!mesh.isReadable)
        {
            Debug.LogWarning("Mesh " + mr.name + " is not readable and be ignored");
            return;
        }

        if(flags.get_xforms)
        {
            var data = default(XformData);
            data.obj_path = msCreateString(mr.name);
            Capture(ref data, mr.GetComponent<Transform>());
            msServerAddServeData(m_server, EventType.Xform, ref data);
            msDeleteString(data.obj_path);
        }
        if (flags.get_meshes)
        {
            var data = default(MeshData);
            data.obj_path = msCreateString(mr.name);
            data.transform = mr.GetComponent<Transform>().localToWorldMatrix;
            Capture(ref data, mesh, null, flags);
            msServerAddServeData(m_server, EventType.Mesh, ref data);
            msDeleteString(data.obj_path);
        }
    }

    void ServeData(SkinnedMeshRenderer smr, GetFlags flags)
    {
        if (flags.get_xforms)
        {
            var data = default(XformData);
            data.obj_path = msCreateString(smr.name);
            Capture(ref data, smr.GetComponent<Transform>());
            msServerAddServeData(m_server, EventType.Xform, ref data);
            msDeleteString(data.obj_path);
        }
        if (flags.get_meshes)
        {
            var data = default(MeshData);

            if(flags.mesh_bake_skin)
            {
                Cloth cloth = smr.GetComponent<Cloth>();
                if (cloth != null)
                {
                    var mesh = smr.sharedMesh;
                    if (mesh == null)
                    {
                        return;
                    }
                    if (!mesh.isReadable)
                    {
                        Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
                        return;
                    }
                    Capture(ref data, mesh, cloth, flags);
                }
                else
                {
                    var mesh = new Mesh();
                    smr.BakeMesh(mesh);
                    Capture(ref data, mesh, null, flags);
                }
            }
            else
            {
                var mesh = smr.sharedMesh;
                if (mesh == null) return;
                if (!mesh.isReadable)
                {
                    Debug.LogWarning("Mesh " + smr.name + " is not readable and be ignored");
                    return;
                }
                Capture(ref data, mesh, null, flags);
            }

            data.obj_path = msCreateString(smr.name);
            data.transform = smr.GetComponent<Transform>().localToWorldMatrix;

            msServerAddServeData(m_server, EventType.Mesh, ref data);
            msDeleteString(data.obj_path);
        }
    }

    void Capture(ref XformData data, Transform trans)
    {
        data.position = trans.localPosition;
        data.rotation = trans.localRotation;
        data.position = trans.localPosition;
    }

    void Capture(ref MeshData data, Mesh mesh, Cloth cloth, GetFlags flags)
    {
        bool use_cloth = cloth != null;

        if(flags.mesh_get_points)
        {
            m_points = use_cloth ? cloth.vertices : mesh.vertices;
            data.points = RawPtr(m_points);
            data.num_points = m_points.Length;
        }
        if (flags.mesh_get_normals)
        {
            m_normals = use_cloth ? cloth.normals : mesh.normals;
            data.normals = RawPtr(m_normals);
            data.num_points = m_normals.Length;
        }
        if (flags.mesh_get_tangents)
        {
            m_tangents = mesh.tangents;
            data.tangents = RawPtr(m_tangents);
            data.num_points = m_tangents.Length;
        }
        if (flags.mesh_get_uv)
        {
            m_uv = mesh.uv;
            data.uv = RawPtr(m_uv);
            data.num_points = m_uv.Length;
        }
        if (flags.mesh_get_indices)
        {
            m_indices = mesh.triangles;
            data.indices = RawPtr(m_indices);
            data.num_indices = m_indices.Length;
        }
    }



#if UNITY_EDITOR
    void Reset()
    {
        // force disable batching for export
        var method = typeof(UnityEditor.PlayerSettings).GetMethod("SetBatchingForPlatform", BindingFlags.NonPublic | BindingFlags.Static);
        if(method != null)
        {
            method.Invoke(null, new object[] { BuildTarget.StandaloneWindows, 0, 0 });
            method.Invoke(null, new object[] { BuildTarget.StandaloneWindows64, 0, 0 });
            method.Invoke(null, new object[] { BuildTarget.StandaloneLinux, 0, 0 });
            method.Invoke(null, new object[] { BuildTarget.StandaloneLinux64, 0, 0 });
            method.Invoke(null, new object[] { BuildTarget.StandaloneLinuxUniversal, 0, 0 });
            method.Invoke(null, new object[] { BuildTarget.StandaloneOSXIntel64, 0, 0 });
            method.Invoke(null, new object[] { BuildTarget.StandaloneOSXUniversal, 0, 0 });
        }
    }
#endif

    void Awake()
    {
        StartServer();
#if UNITY_EDITOR
        EditorApplication.update += PollServerEvents;
#endif
    }

    void OnDestroy()
    {
#if UNITY_EDITOR
        EditorApplication.update -= PollServerEvents;
#endif
        StopServer();
    }


#if UNITY_EDITOR
    bool m_isCompiling;
#endif

    void Update()
    {
#if UNITY_EDITOR
        if (EditorApplication.isCompiling && !m_isCompiling)
        {
            // on compile begin
            m_isCompiling = true;
            StopServer();
        }
        else if (!EditorApplication.isCompiling && m_isCompiling)
        {
            // on compile end
            m_isCompiling = false;
            StartServer();
        }
#endif
        PollServerEvents();
    }
    #endregion
}
