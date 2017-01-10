using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
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
        return Marshal.PtrToStringAnsi(cstring);
    }

    public class MeshData
    {
        public Vector3[] points;
        public Vector3[] normals;
        public Vector4[] tangents;
        public Vector2[] uv;
        public int[] indices;
    }
    #endregion


    #region fields
    [SerializeField] bool m_genNormals = true;
    [SerializeField] bool m_genTangents = false;
    [SerializeField] int m_port = 8080;
    IntPtr m_server;
    msEventHandler m_handler;

    MeshData m_data = new MeshData();
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
            m_data.points = new Vector3[edata.num_points];
            mdata.points = RawPtr(m_data.points);
        }
        if (edata.normals != IntPtr.Zero)
        {
            m_data.normals = new Vector3[edata.num_points];
            mdata.normals = RawPtr(m_data.normals);
        }
        if (edata.tangents != IntPtr.Zero)
        {
            m_data.tangents = new Vector4[edata.num_points];
            mdata.tangents = RawPtr(m_data.tangents);
        }
        if (edata.uv != IntPtr.Zero)
        {
            m_data.uv = new Vector2[edata.num_points];
            mdata.uv = RawPtr(m_data.uv);
        }
        if (edata.indices != IntPtr.Zero)
        {
            m_data.indices = new int[edata.num_indices];
            mdata.indices = RawPtr(m_data.indices);
        }
        msCopyEditData(ref mdata, ref edata);

    }

    public void PollServerEvents()
    {
        msServerProcessEvents(m_server, m_handler);
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
