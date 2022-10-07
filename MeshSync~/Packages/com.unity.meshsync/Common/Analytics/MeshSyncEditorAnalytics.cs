#if UNITY_EDITOR
using UnityEditor;

namespace Unity.MeshSync.Common.Analytics {
internal class MeshSyncEditorAnalytics : IMeshSyncAnalytics {
    private const string VENDORKEY = "unity.meshsync";

        private const string DCCCONFIGURE_INSTALLEDEVENTNAME = "dccConfigure";

        // Data schema version for install event
        private const int DCCCONFIGURE_VERSION = 4;

        private const string MESHSYNC_SYNC_INSTALLEDEVENTNAME = "meshSync_Sync";

        // Data schema version for sync version
        private const int MESHSYNC_SYNC_VERSION = 2;

        private const int DCCCONFIGURE_EVENTS_PER_HOUR = 1000;
        private const int MESHSYNC_SYNC_EVENTS_PER_HOUR = 20000;

        private struct DCCInstallEventData {
            public string meshSyncDccName;
        }



        static MeshSyncEditorAnalytics() {
            EditorAnalytics.RegisterEventWithLimit(DCCCONFIGURE_INSTALLEDEVENTNAME, DCCCONFIGURE_EVENTS_PER_HOUR, 10, VENDORKEY, DCCCONFIGURE_VERSION);
            EditorAnalytics.RegisterEventWithLimit(MESHSYNC_SYNC_INSTALLEDEVENTNAME, MESHSYNC_SYNC_EVENTS_PER_HOUR, 10, VENDORKEY, MESHSYNC_SYNC_VERSION);
        }

        public void UserInstalledPlugin(string dccTool) {
            var data = new DCCInstallEventData { meshSyncDccName = dccTool.ToLower() };
            EditorAnalytics.SendEventWithLimit(DCCCONFIGURE_INSTALLEDEVENTNAME, data, DCCCONFIGURE_VERSION);
        }

        public void UserSyncedData(SyncEventData eventData) {


            EditorAnalytics.SendEventWithLimit(MESHSYNC_SYNC_INSTALLEDEVENTNAME, eventData, MESHSYNC_SYNC_VERSION);
        }
}
}
#endif