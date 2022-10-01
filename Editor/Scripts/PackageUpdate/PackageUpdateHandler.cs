using Unity.FilmInternalUtilities.Editor;
using Unity.MeshSync;
using Unity.MeshSync.Editor;
using UnityEditor;
using UnityEditor.PackageManager;
using UnityEditor.SceneManagement;
using Object = UnityEngine.Object;

[InitializeOnLoad]
public class PackageUpdateHandler
{
    static PackageUpdateHandler() {
        Events.registeredPackages -= OnPackageRegistered;
        Events.registeredPackages += OnPackageRegistered;

        Events.registeringPackages -= OnPackageRegistering;
        Events.registeringPackages += OnPackageRegistering;
    }

    private static void OnPackageRegistering(PackageRegistrationEventArgs obj) {
        if (obj.changedFrom.FindPackage("com.unity.meshsync") == null)
            return;
        
        StopAllServers();
    }
    
    private static void OnPackageRegistered(PackageRegistrationEventArgs obj) {
        
        if (obj.changedTo.FindPackage("com.unity.meshsync") == null)
            return;
        
        // Save the current open scenes
        EditorSceneManager.SaveOpenScenes();
        
        // If the package was just updated, stop all servers and restart the editor
        StopAllServers();

        var path = AssetEditorUtility.GetApplicationRootPath();
        EditorApplication.OpenProject(path);
    }

    private static void StopAllServers() {
        
        // Stop editor server
        EditorServer.StopServer();
        
        // Stop scene servers
        var servers = Object.FindObjectsOfType<MeshSyncServer>();
        foreach (var server in servers) {
            
            server.StopServer();
        }
    }
}
