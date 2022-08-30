using System;
using System.Collections;
using System.Collections.Generic;

using UnityEditor;

using UnityEngine;

namespace Unity.MeshSync.Editor.Analytics {

    public static class MeshSyncAnalyticsFactory {

        public static IMeshSyncAnalytics CreateAnalytics() {
            return new MeshSyncAnalytics();
        }
    }

    public interface IMeshSyncAnalytics {

        void UserInstalledPlugin(string dccTool);
    }

    internal sealed class MeshSyncAnalytics : IMeshSyncAnalytics {
        private const string VENDORKEY = "unity.meshsync";
        private const string INSTALLEDEVENTNAME = "dccConfigure";
        private const int VERSION = 4;

        private struct DCCInstallEventData {
            public string meshSyncDccName;
        }

        static MeshSyncAnalytics() {
            EditorAnalytics.RegisterEventWithLimit(INSTALLEDEVENTNAME, 1000, 10, VENDORKEY, VERSION);
        }

        public void UserInstalledPlugin(string dccTool) {
            var data = new DCCInstallEventData { meshSyncDccName = dccTool.ToLower() };
            EditorAnalytics.SendEventWithLimit(INSTALLEDEVENTNAME, data, VERSION);
        }
    }
}