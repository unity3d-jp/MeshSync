using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEditor.PackageManager;
using UnityEngine;
using PackageInfo = UnityEditor.PackageManager.PackageInfo;


namespace Unity.MeshSync.Editor {

internal static class PackageEventSubscriber {

    [InitializeOnLoadMethod]
    private static void PackageEventSubscriber_OnEditorLoad() {

        Events.registeringPackages -= OnPackageRegistering;
        Events.registeringPackages += OnPackageRegistering;
            

        Events.registeredPackages -= OnPackagesRegistered;
        Events.registeredPackages += OnPackagesRegistered;
    }
    //---------------------------------------------------------------------------------------------------------------------

    static void OnPackageRegistering(PackageRegistrationEventArgs packageRegistrationEventArgs) {
        var package = packageRegistrationEventArgs.changedFrom.FindPackage(MeshSyncConstants.PACKAGE_NAME);
        if (package == null)
            return;
        
        StopAllServers();
    }
    //----------------------------------------------------------------------------------------------------------------------    
    
    static void OnPackagesRegistered(PackageRegistrationEventArgs packageRegistrationEventArgs) {
       
        PackageInfo curPackage = packageRegistrationEventArgs.removed.FindPackage(MeshSyncConstants.PACKAGE_NAME);

        if (null == curPackage) {
            curPackage = packageRegistrationEventArgs.changedTo.FindPackage(MeshSyncConstants.PACKAGE_NAME);
        }

        if (null == curPackage) {
            return;
        }
        
        StopAllServers();
        
        EditorRestartMessageNotifier.RequestNotificationOnLoad(curPackage);                    
    }
    //---------------------------------------------------------------------------------------------------------------------- 
    
    static void StopAllServers() {

        // Stop editor server
        EditorServer.StopSession();

        // Stop scene servers
        var servers = Object.FindObjectsOfType<MeshSyncServer>();
        foreach (var server in servers) {

            server.StopSession();
        }
    }
}

} //end namespace
