using System;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEditorInternal;
using UnityEngine;
using UnityEngine.SceneManagement;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor {

[InitializeOnLoad]
public static class AutoServerCreation{

    static AutoServerCreation() {
        EditorApplication.update += UpdateCall;
    }

    private static void UpdateCall() {
        EditorApplication.update -= UpdateCall;
        
        if(AutoServerCreationSettings.instance.HasPromptedUser)
            return;
        
        AutoServerCreationSettings.instance.HasPromptedUser = true;
        PromptUser();
    }

    private static void PromptUser() {
        if (Object.FindObjectOfType<MeshSyncServer>() != null)
            return;

        if (!EditorUtility.DisplayDialog(
            "Create MeshSync Server",
            "The current scene does not have a MeshSync Server. Would you like to create one?",
            "Yes", "No"))
            return;
        
        MeshSyncMenu.CreateMeshSyncServer(autoStart: true);
        EditorSceneManager.MarkSceneDirty(SceneManager.GetActiveScene());
    }

}
}
