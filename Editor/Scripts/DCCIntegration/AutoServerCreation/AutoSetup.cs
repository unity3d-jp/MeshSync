using System;
using System.Threading;
using UnityEditor;
using UnityEngine;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor {

[InitializeOnLoad]
internal static class AutoSetup {

    private static string m_appRootPath = null;
    
    //[TODO-sin] replace with FilmInternalUtilities.Editor.AssetEditorUtility.GetApplicationRootPath
    static string GetApplicationRootPath() {
        if (null != m_appRootPath)
            return m_appRootPath;

        //Not using Application.dataPath because it may not be called in certain times, e.g: during serialization
        
        m_appRootPath = System.IO.Directory.GetCurrentDirectory().Replace('\\','/');
        return m_appRootPath;
    }

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
        var path = GetApplicationRootPath();
        m_server.NotifyEditorCommand(path, message);
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
