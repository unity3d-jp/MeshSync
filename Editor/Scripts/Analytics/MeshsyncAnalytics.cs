using UnityEngine;

namespace Unity.MeshSync.Editor.Analytics {
internal static class MeshSyncAnalyticsFactory {
    public static IMeshSyncAnalytics CreateAnalytics() {
        return new MeshSyncAnalytics();
    }
}

internal interface IMeshSyncAnalytics {
    /// <summary>
    /// Send an event that a user installed and configured a DCC tool for use with MeshSync
    /// </summary>
    /// <param name="dccTool">Short designation for the DCC which was installed and configured</param>
    void UserInstalledPlugin(string dccTool);

    void UserSyncedData(MeshSyncAnalyticsData data);
}

internal sealed class MeshSyncAnalytics : IMeshSyncAnalytics {
    static MeshSyncAnalytics() {
        AnalyticsHandlerFactory.RegisterHandlers();
    }

    public void UserInstalledPlugin(string dccTool) {
        new InstallAnalyticsHandler().Send(dccTool);
    }

    public void UserSyncedData(MeshSyncAnalyticsData data) {
        AnalyticsDataHandler<MeshSyncAnalyticsData> handler = AnalyticsHandlerFactory.GetHandler(data);

        if (handler == null) {
            Debug.LogError("No analytics handler found!");
            return;
        }

        handler.Send(data);
    }
}
}