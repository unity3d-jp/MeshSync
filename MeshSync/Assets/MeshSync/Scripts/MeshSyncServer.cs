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
    public struct ServerSettings
    {
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
    #endregion


    #region fields
    [SerializeField] int m_port = 8080;
    IntPtr m_server;
    msEventHandler m_handler;
    #endregion



    #region impl
    void StartServer()
    {
        StopServer();

        var settings = ServerSettings.default_value;
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
                Debug.Log("OnServerEvent(): Edit");
                break;
        }
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
