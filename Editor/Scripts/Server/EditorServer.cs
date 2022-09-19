using System;
using UnityEditor;
using UnityEngine;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor {

[InitializeOnLoad]
internal static class EditorServer {

    private static Server            m_server;

    private const string CONFIGURATION_TIP =
        "You can configure the editor server in Edit/ProjectSettings/MeshSync/EditorServer";

    static EditorServer() {
        ApplySettingsIfDirty();
    }
    
    internal static void ApplySettingsIfDirty() {
        
        if (!EditorServerSettings.instance.Dirty)
            return;
        
        ApplySettings();
        
        EditorServerSettings.instance.Dirty = false;
    }

    private static void ApplySettings() {
        
        EditorApplication.update -= UpdateCall;
        m_server.Stop();
        
        if (!EditorServerSettings.instance.Active) {
            Debug.Log("[MeshSync] Stopping Editor Server.\n" + CONFIGURATION_TIP);
            
            return;
        }
        
        EditorApplication.update += UpdateCall;
        
        var settings = ServerSettings.defaultValue;
        settings.port = EditorServerSettings.instance.Port;

        if (!Server.Start(ref settings, out m_server)) {
            EditorUtility.DisplayDialog(
                "Server Error",
                $"Could not start Editor Server in port {settings.port}.\n"+CONFIGURATION_TIP,
                "Ok");
            
            Debug.LogErrorFormat("[MeshSync] Could not start editor server at port {0}\n" + CONFIGURATION_TIP, settings.port);
            return;
        }
        
        Debug.LogFormat("[MeshSync] Starting Editor Server at port {0}.\n" + CONFIGURATION_TIP, settings.port);
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
        var type = message.commandType;
        
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
        if (AddServerToScene()) {
            m_server.NotifyEditorCommand("ok", message);
        }
        else {
            m_server.NotifyEditorCommand("Could not start server", message);
            Debug.LogErrorFormat("[MeshSync] Could not add server to scene");
        }
    }

    private static void HandleGetProjectPath(EditorCommandMessage message) {
        var path = GetProjectPath();
        m_server.NotifyEditorCommand(path, message);
    }

    private static string GetProjectPath() {
        var path = Application.dataPath;
        path = path.Replace("/Assets", "");
        return path;
    }
    
    private static bool AddServerToScene() {
        //check if the scene has a server
        var servers = Object.FindObjectsOfType<MeshSyncServer>();
        if (servers.Length > 0)
            return true;
        
        var server = MeshSyncMenu.CreateMeshSyncServer(true);
        return server.IsServerStarted();
    }
}
}
