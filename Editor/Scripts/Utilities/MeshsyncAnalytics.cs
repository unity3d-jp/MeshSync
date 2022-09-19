using System;
using System.Collections;
using System.Collections.Generic;

using UnityEditor;

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

        void UserSyncedData(AssetType type);
    }

    internal sealed class MeshSyncAnalytics : IMeshSyncAnalytics {
        private const string VENDORKEY = "unity.meshsync";

        private const string DCCCONFIGURE_INSTALLEDEVENTNAME = "dccConfigure";
        private const int DCCCONFIGURE_VERSION = 4;

        private const string MESHSYNC_SYNC_INSTALLEDEVENTNAME = "meshSync_Sync";
        private const int MESHSYNC_SYNC_VERSION = 1;
        private struct DCCInstallEventData {
            public string meshSyncDccName;
        }

        private struct SyncEventData {
            public string assetSyncType;
        }

        static MeshSyncAnalytics() {
            EditorAnalytics.RegisterEventWithLimit(DCCCONFIGURE_INSTALLEDEVENTNAME, 1000, 10, VENDORKEY, DCCCONFIGURE_VERSION);
            EditorAnalytics.RegisterEventWithLimit(MESHSYNC_SYNC_INSTALLEDEVENTNAME, 1000, 10, VENDORKEY, MESHSYNC_SYNC_VERSION);
        }

        public void UserInstalledPlugin(string dccTool) {
            var data = new DCCInstallEventData { meshSyncDccName = dccTool.ToLower() };
            EditorAnalytics.SendEventWithLimit(DCCCONFIGURE_INSTALLEDEVENTNAME, data, DCCCONFIGURE_VERSION);
        }

        public void UserSyncedData(AssetType type) {
            var data = new SyncEventData {
                assetSyncType = type.ToString().ToLower()
            };

            EditorAnalytics.SendEventWithLimit(MESHSYNC_SYNC_INSTALLEDEVENTNAME, data, MESHSYNC_SYNC_VERSION);
        }
    }
}