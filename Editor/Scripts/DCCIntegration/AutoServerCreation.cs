using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[InitializeOnLoad]
public static class AutoServerCreation {
    
    static AutoServerCreation() {
        
        if (Object.FindObjectOfType<MeshSyncServer>() != null)
            return;

        if (EditorUtility.DisplayDialog(
            "Create Server",
            "The current scene does not have a MeshSync Server. Would like to create one?",
            "Ignore",
            "Create",
            DialogOptOutDecisionType.ForThisSession,
            MeshSyncConstants.OPT_OUT_AUTO_CREATE_SERVER))
            return;
        
        MeshSyncMenu.CreateMeshSyncServer(autoStart:true);
    }
}
}
