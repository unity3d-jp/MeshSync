using System;
using System.Threading;
using UnityEditor;
using UnityEngine;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor {

[InitializeOnLoad]
public static class AutoSetup {

    private static Server m_server;
    
    static AutoSetup() {
        // Create server
        var settings = ServerSettings.defaultValue;
        
        settings.port = AutoSetupSettings.instance.Port;
        
        if (!Server.Start(ref settings, out m_server)) {
            Debug.LogError("[MeshSync] Could not start Server");
            return;
        }
        
        //Register calls
        EditorApplication.update -= UpdateCall;
        EditorApplication.update += UpdateCall;
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
                HandleAddServerToScene();
                break;
            case EditorCommandMessage.CommandType.GetProjectPath:
                HandleGetProjectPath();
                break;
            default:
                Debug.LogErrorFormat("[MeshSync] Unhandled command type {0}", type);
                break;
        }
    }

    private static void HandleAddServerToScene() {
        AddServerToScene();
        
        m_server.NotifyEditorCommand(EditorCommandMessage.CommandType.AddServerToScene, "ok");
    }

    private static void HandleGetProjectPath() {
        var path = GetProjectPath();
        m_server.NotifyEditorCommand(EditorCommandMessage.CommandType.GetProjectPath, path);
    }

    private static string GetProjectPath() {
        var path = Application.dataPath;
        path = path.Replace("/Assets", "");
        return path;
    }
    
    private static void AddServerToScene() {
        //check if the scene has a server
        var servers = Object.FindObjectsOfType<MeshSyncServer>();
        if (servers.Length > 0)
            return;
        
        var server = MeshSyncMenu.CreateMeshSyncServer(true);
        while (!server.IsServerStarted()) {
            Thread.Sleep(100);
        }
    }
}
}
