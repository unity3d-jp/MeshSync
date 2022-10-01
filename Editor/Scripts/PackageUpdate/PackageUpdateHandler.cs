using System;
using System.Threading;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEditor.PackageManager;
using UnityEditor.SceneManagement;
using UnityEngine;
using Object = UnityEngine.Object;


namespace Unity.MeshSync.Editor{

[InitializeOnLoad]
internal class PackageUpdateHandler {
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
           
        // If the package was just updated, stop all servers and restart the editor
        StopAllServers();        

        // Save the current open scenes
        EditorSceneManager.SaveOpenScenes();

        var path = AssetEditorUtility.GetApplicationRootPath();
        EditorApplication.OpenProject(path);
    }

    private static void StopAllServers() {

        // Stop editor server
        EditorServer.StopSession();

        // Stop scene servers
        var servers = Object.FindObjectsOfType<MeshSyncServer>();
        foreach (var server in servers) {

            server.StopSession();
        }
    }
}
}
