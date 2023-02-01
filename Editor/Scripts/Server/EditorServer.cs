using System;
using System.IO;
using System.Text;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor {
[InitializeOnLoad]
internal static class EditorServer {
    private const string APPLIED_SETTINGS_KEY = "MESHSYNC_EDITOR_SERVER_APPLIED_DEFAULT_SETTINGS";
    private const string PORT_KEY             = "MESHSYNC_EDITOR_SERVER_PORT";
    private const string ACTIVE_KEY           = "MESHSYNC_EDITOR_ACTIVE";
    private const string ACTIVE_PREV_KEY      = "MESHSYNC_EDITOR_ACTIVE_PREV";
    private const string CONFIGURATION_TIP    = "You can configure the editor server via Project Settings";
    private const string CLI_ARGUMENT_PORT    = "PORT";
    private const string CLI_ARGUMENT_ACTIVE  = "SERVER_ACTIVE";

    private static void UpdateLog() {
        string appRoot = AssetEditorUtility.GetApplicationRootPath();
        string dir     = Path.Combine(appRoot, "Logs");
        Directory.CreateDirectory(dir);
        string path = Path.Combine(dir, "MeshSyncEditorServerLog.txt");
        using (FileStream stream = File.Create(path)) {
            string log   = $"active:{Active}\nport:{Port}";
            byte[] bytes = new UTF8Encoding(true).GetBytes(log);
            stream.Write(bytes, 0, bytes.Length);
        }
    }

    internal static bool Active {
        get { return SessionState.GetBool(ACTIVE_KEY, false); }
        set {
            SessionState.SetBool(ACTIVE_KEY, value);
            UpdateLog();
        }
    }

    private static bool ActivePrev {
        get { return SessionState.GetBool(ACTIVE_PREV_KEY, false); }
        set { SessionState.SetBool(ACTIVE_PREV_KEY, value); }
    }

    internal static ushort Port {
        get { return (ushort)SessionState.GetInt(PORT_KEY, 8081); }
        set {
            SessionState.SetInt(PORT_KEY, value);
            UpdateLog();
        }
    }

    private static bool AppliedInitialSettings {
        get { return SessionState.GetBool(APPLIED_SETTINGS_KEY, false); }
        set { SessionState.SetBool(APPLIED_SETTINGS_KEY, value); }
    }

    private static Server m_server;


    static EditorServer() {
        // To avoid timeouts where the server cannot react due to the editor loading
        // Defer the Initialisation to the first update call
        EditorApplication.update -= Init;
        EditorApplication.update += Init;

        EditorApplication.quitting -= OnQuit;
        EditorApplication.quitting += OnQuit;
    }

    private static void OnQuit() {
        m_server.Abort();
    }

    /// <summary>
    /// Apply settings from CLI arguments or use Editor Server Settings.
    /// </summary>
    private static void ApplyInitialSettings() {
        if (AppliedInitialSettings)
            return;

        AppliedInitialSettings = true;

        string[] arguments = Environment.GetCommandLineArgs();

        int activeKeyIndex = Array.IndexOf(arguments, CLI_ARGUMENT_ACTIVE) + 1;
        Active = activeKeyIndex > 0 ? bool.Parse(arguments[activeKeyIndex]) : EditorServerSettings.instance.Active;

        int portKeyIndex = Array.IndexOf(arguments, CLI_ARGUMENT_PORT) + 1;
        Port = portKeyIndex > 0 ? ushort.Parse(arguments[portKeyIndex]) : EditorServerSettings.instance.Port;
        ApplySettings();
    }

    internal static void ApplySettings() {
        DoApplySettings();
        ActivePrev = Active;
    }

    private static void DoApplySettings() {
        EditorApplication.update -= UpdateCall;
        m_server.Stop();

        if (!Active) {
            if (ActivePrev) Debug.Log("[MeshSync] Stopping Editor Server.\n" + CONFIGURATION_TIP);
            return;
        }

        EditorApplication.update += UpdateCall;

        ServerSettings settings = ServerSettings.defaultValue;
        settings.port = Port;

        if (!Server.Start(ref settings, out m_server)) {
            EditorUtility.DisplayDialog(
                "Server Error",
                $"Could not start Editor Server in port {settings.port}.\n" + CONFIGURATION_TIP,
                "Ok");

            Debug.LogErrorFormat("[MeshSync] Could not start editor server at port {0}\n" + CONFIGURATION_TIP, settings.port);
            return;
        }

        Debug.LogFormat("[MeshSync] Starting Editor Server at port {0}.\n" + CONFIGURATION_TIP, settings.port);
    }

    private static void Init() {
        EditorApplication.update -= Init;
        ApplyInitialSettings();
    }

    private static void UpdateCall() {
        if (m_server.numMessages == 0)
            return;

        m_server.ProcessMessages(Handler);
    }

    private static void Handler(NetworkMessageType type, IntPtr data) {
        switch (type) {
            case NetworkMessageType.EditorCommand:
                OnRecvEditorCommand((EditorCommandMessage)data);
                break;
            default:
                Debug.LogErrorFormat("[MeshSync] Unhandled message type {0}", type);
                break;
        }
    }

    private static void OnRecvEditorCommand(EditorCommandMessage message) {
        EditorCommandMessage.CommandType type = message.commandType;
        switch (type) {
            case EditorCommandMessage.CommandType.AddServerToScene:
                HandleAddServerToScene(message);
                break;
            case EditorCommandMessage.CommandType.GetProjectPath:
                HandleGetProjectPath(message);
                break;
            default:
                Debug.LogErrorFormat("[MeshSync] Unhandled command type {0}", type);
                break;
        }
    }

    private static void HandleAddServerToScene(EditorCommandMessage message) {
        int port = int.Parse(message.buffer);
        if (AddServerToScene(port)) {
            m_server.NotifyEditorCommand("ok", message);
        }
        else {
            m_server.NotifyEditorCommand("Could not start server with port " + port, message);
            Debug.LogErrorFormat("[MeshSync] Could not add server to scene");
        }
    }

    private static void HandleGetProjectPath(EditorCommandMessage message) {
        string path = AssetEditorUtility.GetApplicationRootPath();
        m_server.NotifyEditorCommand(path, message);
    }

    private static bool AddServerToScene(int port) {
        //check if the scene has a server
        MeshSyncServer[] servers = Object.FindObjectsOfType<MeshSyncServer>();
        foreach (MeshSyncServer server in servers)
            if (server.GetServerPort() == port)
                return true;

        MeshSyncServer newServer = CreateServer(port);
        return newServer.IsServerStarted();
    }

    private static MeshSyncServer CreateServer(int port) {
        GameObject     go  = new GameObject("MeshSyncServer");
        MeshSyncServer mss = go.AddComponent<MeshSyncServer>();
        mss.Init(MeshSyncConstants.DEFAULT_ASSETS_PATH);
        mss.SetServerPort(port);
        mss.SetAutoStartServer(true);
        return mss;
    }
}
}