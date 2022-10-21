using System;
using System.Linq;
using UnityEditor;

using UnityEngine;
using UnityEngine.Analytics;

namespace Unity.MeshSync.Editor.Analytics {
    /// <summary>
    /// Factory class to create the required analytics handler for the given MeshSyncAnalyticsData.
    /// </summary>
    internal static class AnalyticsHandlerFactory {
        public static void RegisterHandlers() {
            // Finds subclasses of AnalyticsHandlerFactory and calls Register() on them all:
            var analyticHandlerSubclasses = typeof(AnalyticsHandlerFactory).Assembly.GetTypes().Where(type => type.IsSubclassOf(typeof(AnalyticsDataHandler)));
            foreach (Type analyticHandlerSubclass in analyticHandlerSubclasses) {
                var t = (AnalyticsDataHandler)Activator.CreateInstance(analyticHandlerSubclass);
                t.Register();
            }
        }

        public static AnalyticsDataHandler GetHandler(MeshSyncAnalyticsData data) {
            if (data.sessionStartData.HasValue) {
                return new SessionStartAnalyticsHandler();
            }

            if (data.syncData.HasValue) {
                return new SyncAnalyticsHandler();
            }

            // TODO: If new handlers are added, update this to return one depending on the field it uses:

            return null;
        }
    }

    /// <summary>
    /// Base class for all analytics handlers.
    /// </summary>
    internal abstract class AnalyticsDataHandler {

        protected virtual string eventName { get; }
        protected virtual int version { get; }
        protected virtual int eventsPerHour => 10000;

        private static void logIfWarning(AnalyticsResult resp) {
            if (resp != AnalyticsResult.Ok) {
                Debug.LogWarning($"Analytics endpoint reported: {resp} when should be {AnalyticsResult.Ok}");
            }
        }
        protected void Send(object data) {
            logIfWarning(
                EditorAnalytics.SendEventWithLimit(
                    eventName,
                        data,
                    version));
        }

        public abstract void Send(MeshSyncAnalyticsData data);

        public void Register() {
            logIfWarning(
                EditorAnalytics.RegisterEventWithLimit(
                    eventName,
                    eventsPerHour,
                    10,
                    MeshSyncAnalytics.VENDORKEY,
                    version));

        }
    }

    /// <summary>
    /// Handles MeshSyncAnalyticsData.sessionStartData
    /// </summary>
    internal class SessionStartAnalyticsHandler : AnalyticsDataHandler {
        internal class SessionEventData {
            public string dccToolName;
        }

        protected override string eventName => "meshSync_session_start";
        protected override int version => 2;

        public override void Send(MeshSyncAnalyticsData data) {
            var sessionStartData = data.sessionStartData.Value;
            var eventData = new SessionEventData {
                dccToolName = sessionStartData.DCCToolName
            };
            
            Send(eventData);
        }
    }

    /// <summary>
    /// Handles MeshSyncAnalyticsData.syncData
    /// </summary>
    internal class SyncAnalyticsHandler : AnalyticsDataHandler {

        private struct SyncEventData {
            public string assetSyncType;
            public string entitySyncType;
            public string syncMode;
        }

        protected override string eventName => "meshSync_Sync";
        protected override int version => 2;

        public override void Send(MeshSyncAnalyticsData data) {
            var syncData = data.syncData.Value;

            var assetTypeStr = syncData.assetType == AssetType.Unknown
                ? "none"
                : syncData.assetType.ToString().ToLower();

            var entityTypeStr = syncData.entityType == EntityType.Unknown
                ? "none"
                : syncData.entityType.ToString().ToLower();

            var eventData = new SyncEventData {
                assetSyncType = assetTypeStr,
                entitySyncType = entityTypeStr,
                syncMode = syncData.syncMode
            };

            Send(eventData);
        }
    }
}
