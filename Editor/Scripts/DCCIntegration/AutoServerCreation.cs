using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[InitializeOnLoad]
public static class AutoServerCreation {
    private const string IS_FIRST_SESSION_CALL = "MeshSync.AutoServerCreation.FirstSessionCall";
    private const string OPT_OUT               = "MeshSync.AutoServerCreation.OptOut";
    
    static AutoServerCreation() {

        if (!SessionState.GetBool(IS_FIRST_SESSION_CALL, true))
            return;
        
        SessionState.SetBool(IS_FIRST_SESSION_CALL, false);
        
        if (Object.FindObjectOfType<MeshSyncServer>() != null)
            return;

        if (!EditorUtility.DisplayDialog(
            "Create MeshSync Server",
            "The current scene does not have a MeshSync Server. Would you like to create one?",
            "No", "Yes", DialogOptOutDecisionType.ForThisMachine, OPT_OUT))
            return;
        
        MeshSyncMenu.CreateMeshSyncServer(autoStart:true);
    }
}
}
