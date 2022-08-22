using System;
using UnityEditor;
using UnityEngine;

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
                AddServerToScene();
                break;
            default:
                Debug.LogErrorFormat("[MeshSync] Unhandled command type {0}", type);
                break;
        }
    }

    private static void AddServerToScene() {
        Debug.LogFormat("[MeshSync] Adding server to scene..");
    }
}
}
