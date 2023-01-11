using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEditor.PackageManager;
using PackageInfo = UnityEditor.PackageManager.PackageInfo;


namespace Unity.MeshSync.Editor {
internal static class PackageEventSubscriber {
    [InitializeOnLoadMethod]
    private static void PackageEventSubscriber_OnEditorLoad() {
        Events.registeredPackages += OnPackagesRegistered;
    }

//----------------------------------------------------------------------------------------------------------------------    

    private static void OnPackagesRegistered(PackageRegistrationEventArgs packageRegistrationEventArgs) {
        PackageInfo curPackage = packageRegistrationEventArgs.removed.FindPackage(MeshSyncConstants.PACKAGE_NAME);

        if (null == curPackage) curPackage = packageRegistrationEventArgs.changedTo.FindPackage(MeshSyncConstants.PACKAGE_NAME);

        if (null == curPackage) return;
        EditorRestartMessageNotifier.RequestNotificationOnLoad(curPackage);
    }
}
} //end namespace