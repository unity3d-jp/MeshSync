using System.Runtime.CompilerServices;

[assembly: InternalsVisibleTo("Unity.MeshSync")]
[assembly: InternalsVisibleTo("Unity.MeshSync.Editor")]

namespace Unity.MeshSync.Common.Analytics {
internal interface IMeshSyncAnalytics {
    /// <summary>
    /// Send an event that a user installed and configured a DCC tool for use with MeshSync
    /// </summary>
    /// <param name="dccTool">Short designation for the DCC which was installed and configured</param>
    void UserInstalledPlugin(string dccTool);

    void UserSyncedData(SyncEventData eventData);
}
}