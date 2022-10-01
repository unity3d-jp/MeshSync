using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEditor.PackageManager;
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
        EditorServer.StartSession();
    }

    private static void StopAllServers() {

        // Stop editor server
        EditorServer.StopSession();

        // Stop scene servers
        var servers = Object.FindObjectsOfType<MeshSyncServer>();
        foreach (var server in servers) {

            server.StopServer();
        }
    }
}
}
