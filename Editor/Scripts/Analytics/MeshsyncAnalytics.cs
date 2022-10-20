using UnityEditor;

using UnityEngine;
using UnityEngine.Analytics;

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
        private const string VENDORKEY = "unity.meshsync";

        private const string DCCCONFIGURE_INSTALLEDEVENTNAME = "dccConfigure";

        // Data schema version for install event
        private const int DCCCONFIGURE_VERSION = 4;

        private const string MESHSYNC_SYNC_INSTALLEDEVENTNAME = "meshSync_Sync";

        // Data schema version for sync version
        private const int MESHSYNC_SYNC_VERSION = 2;

        private const string MESHSYNC_SYNC_SESSIONSTARTEDEVENTNAME = "session_start";
        // Data schema version for session_start
        private const int MESHSYNC_SYNC_SESSIONSTART_VERSION = 1;

        private const int DCCCONFIGURE_EVENTS_PER_HOUR = 1000;
        private const int MESHSYNC_SYNC_EVENTS_PER_HOUR = 10000;

        private struct DCCInstallEventData {
            public string meshSyncDccName;
        }

        private struct SyncEventData {
            public string assetSyncType;
            public string entitySyncType;
            public string syncMode;
        }
        
        private struct SessionEventData {
            public string dccToolName;
        }

        private static void logIfWarning(AnalyticsResult resp) {
            if (resp != AnalyticsResult.Ok) {
                Debug.LogWarning($"Analytics endpoint reported: {resp} when should be {AnalyticsResult.Ok}");
            }
        }

        static MeshSyncAnalytics() {
            logIfWarning(
                EditorAnalytics.RegisterEventWithLimit(
                    DCCCONFIGURE_INSTALLEDEVENTNAME,
                    DCCCONFIGURE_EVENTS_PER_HOUR,
                    10,
                    VENDORKEY,
                    DCCCONFIGURE_VERSION));

            logIfWarning(
                EditorAnalytics.RegisterEventWithLimit(
                    MESHSYNC_SYNC_INSTALLEDEVENTNAME,
                    MESHSYNC_SYNC_EVENTS_PER_HOUR,
                    10,
                    VENDORKEY,
                    MESHSYNC_SYNC_VERSION));
        }

        public void UserInstalledPlugin(string dccTool) {
            var data = new DCCInstallEventData { meshSyncDccName = dccTool.ToLower() };
            logIfWarning(EditorAnalytics.SendEventWithLimit(DCCCONFIGURE_INSTALLEDEVENTNAME, data, DCCCONFIGURE_VERSION));
        }

        public void UserSyncedData(MeshSyncAnalyticsData data) {
            if (data.syncData.HasValue) {
                var syncData = data.syncData.Value;

                var assetTypeStr = syncData.assetType == AssetType.Unknown
                    ? "none"
                    : syncData.assetType.ToString().ToLower();

                var entityTypeStr = syncData.entityType == EntityType.Unknown
                    ? "none"
                    : syncData.entityType.ToString().ToLower();

                var eventData = new SyncEventData {
                    assetSyncType  = assetTypeStr,
                    entitySyncType = entityTypeStr,
                    syncMode       = syncData.syncMode.ToString()
                };

                logIfWarning(
                    EditorAnalytics.SendEventWithLimit(
                        MESHSYNC_SYNC_INSTALLEDEVENTNAME,
                        eventData,
                        MESHSYNC_SYNC_VERSION));
            }
            else if (data.sessionStartData.HasValue) {
                var sessionStartData = data.sessionStartData.Value;
                var eventData = new SessionEventData {
                    dccToolName = sessionStartData.DCCToolName
                };

                logIfWarning(
                    EditorAnalytics.SendEventWithLimit(
                        MESHSYNC_SYNC_SESSIONSTARTEDEVENTNAME,
                        eventData,
                        MESHSYNC_SYNC_SESSIONSTART_VERSION));
            }
        }
    }
}