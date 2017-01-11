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
        Edit,
    }

    public struct EditData
    {
        public IntPtr obj_path;
        public IntPtr points;
        public IntPtr normals;
        public IntPtr tangents;
        public IntPtr uv;
        public IntPtr indices;
        public int num_points;
        public int num_indices;
        public Vector3 position;
    };


    public struct ServerFlags
    {
        public int flags;
        public bool split
        {
            get { return (flags & 0x1) != 0; }
            set
            {
                if (value) { flags |= 0x1; }
                else { flags &= ~0x1; }
            }
        }
        public bool genNormals
        {
            get { return (flags & 0x2) != 0; }
            set
            {
                if (value) { flags |= 0x2; }
                else { flags &= ~0x2; }
            }
        }
        public bool genTangents
        {
            get { return (flags & 0x4) != 0; }
            set
            {
                if (value) { flags |= 0x4; }
                else { flags &= ~0x4; }
            }
        }
    }

    public struct ServerSettings
    {
        public ServerFlags flags;
        public int max_queue;
        public ushort port;

        public static ServerSettings default_value
        {
            get
            {
                return new ServerSettings
                {
                    max_queue = 100,
                    port = 8080,
                };
            }
        }
    }

    [DllImport("MeshSyncServer")] public static extern IntPtr msServerStart(ref ServerSettings settings);
    [DllImport("MeshSyncServer")] public static extern void msServerStop(IntPtr sv);

    public delegate void msEventHandler(EventType type, IntPtr data);
    [DllImport("MeshSyncServer")] public static extern void msServerProcessEvents(IntPtr sv, msEventHandler handler);

    [DllImport("MeshSyncServer")] public static extern void msCopyEditData(ref EditData dst, ref EditData src);
    [DllImport("MeshSyncServer")] public static extern void msCopyEditData(ref EditData dst, IntPtr src);


    public static IntPtr RawPtr(Array v)
    {
        return v == null ? IntPtr.Zero : Marshal.UnsafeAddrOfPinnedArrayElement(v, 0);
    }
    public static string S(IntPtr cstring)
    {
        return cstring == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(cstring);
    }

    public class MeshData
    {
    }
    #endregion


    #region fields
    [SerializeField] bool m_genNormals = true;
    [SerializeField] bool m_genTangents = false;
    [SerializeField] int m_port = 8080;
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
        settings.flags.genNormals = m_genNormals;
        settings.flags.genTangents = m_genTangents;
        settings.port = (ushort)m_port;
        m_server = msServerStart(ref settings);
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
            case EventType.Edit:
                OnEditEvent(data);
                break;
        }
    }

    void OnEditEvent(IntPtr data)
    {
        var edata = default(EditData);
        msCopyEditData(ref edata, data);


        var mdata = default(EditData);
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
        msCopyEditData(ref mdata, ref edata);


        var create = edata.num_points > 0;
        var path = S(edata.obj_path);
        var names = path.Split('/');
        Transform t = null;
        foreach (var name in names)
        {
            t = FindOrCreateObject(t, name, create);
        }

        if (t == null)
        {
            return;
        }
        if (!create)
        {
            GameObject.Destroy(t.gameObject);
            return;
        }

        var mesh = GetOrAddMeshComponents(t.gameObject);
        mesh.Clear();
        if (edata.points != IntPtr.Zero) { mesh.vertices = m_points; }
        if (edata.normals != IntPtr.Zero) { mesh.normals = m_normals; }
        if (edata.tangents != IntPtr.Zero) { mesh.tangents = m_tangents; }
        if (edata.uv != IntPtr.Zero) { mesh.uv = m_uv; }
        mesh.SetIndices(m_indices, MeshTopology.Triangles, 0);
        mesh.UploadMeshData(false);

        ForceRepaint();
        Debug.Log("MeshSyncServer: Edit " + path);
    }


    Transform FindOrCreateObject(Transform parent, string name, bool create)
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

        if (create && ret == null)
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
        var mfilter = go.GetComponent<MeshFilter>();
        if(mfilter == null)
        {
            mfilter = go.AddComponent<MeshFilter>();
            var mesh = new Mesh();
            mfilter.sharedMesh = mesh;
            mesh.MarkDynamic();

            var renderer = go.AddComponent<MeshRenderer>();
#if UNITY_EDITOR
            Material[] materials = new Material[] { UnityEngine.Object.Instantiate(GetDefaultMaterial()) };
            materials[0].name = "Material_0";
            renderer.sharedMaterials = materials;
#endif
        }
        return mfilter.sharedMesh;
    }

    void ForceRepaint()
    {
#if UNITY_EDITOR
        SceneView.RepaintAll();
#endif
    }



    void Awake()
    {
        m_handler = OnServerEvent;

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

    void Update()
    {
        //PollServerEvents();
    }
    #endregion
}
