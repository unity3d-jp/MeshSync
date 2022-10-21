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
        internal const string VENDORKEY = "unity.meshsync";

        private const string DCCCONFIGURE_INSTALLEDEVENTNAME = "dccConfigure";

        // Data schema version for install event
        private const int DCCCONFIGURE_VERSION = 4;

        private const int DCCCONFIGURE_EVENTS_PER_HOUR = 1000;

        private struct DCCInstallEventData {
            public string meshSyncDccName;
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

            AnalyticsHandlerFactory.RegisterHandlers();
        }

        public void UserInstalledPlugin(string dccTool) {
            var data = new DCCInstallEventData { meshSyncDccName = dccTool.ToLower() };
            logIfWarning(EditorAnalytics.SendEventWithLimit(DCCCONFIGURE_INSTALLEDEVENTNAME, data, DCCCONFIGURE_VERSION));
        }

        public void UserSyncedData(MeshSyncAnalyticsData data) {
            var handler = AnalyticsHandlerFactory.GetHandler(data);

            if (handler == null) {
                Debug.LogError("No analytics handler found!");
                return;
            }

            handler.Send(data);
        }
    }
}