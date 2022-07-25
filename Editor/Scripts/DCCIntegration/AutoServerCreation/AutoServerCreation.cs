using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Unity.MeshSync.Editor {

[InitializeOnLoad]
public static class AutoServerCreation {
    private const string IS_FIRST_SESSION_CALL = "MeshSync.AutoServerCreation.FirstSessionCall";
    private const string OPT_OUT               = "MeshSync.AutoServerCreation.OptOut";
    
    static AutoServerCreation() {
        
        if(AutoServerCreationSettings.instance.HasPromptedUser)
            return;
        
        AutoServerCreationSettings.instance.HasPromptedUser = true;

        EditorApplication.update += Update;
    }

    private static void Update() {
        PromptUser();
        EditorApplication.update -= Update;
    }

    private static void PromptUser() {
        if (Object.FindObjectOfType<MeshSyncServer>() != null)
            return;

        if (EditorUtility.DisplayDialog(
            "Create MeshSync Server",
            "The current scene does not have a MeshSync Server. Would you like to create one?",
            "No", "Yes", DialogOptOutDecisionType.ForThisMachine, OPT_OUT))
            return;
        
        MeshSyncMenu.CreateMeshSyncServer(autoStart: true);
        EditorSceneManager.MarkSceneDirty(SceneManager.GetActiveScene());
    }

}
}
