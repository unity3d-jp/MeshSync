using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using UnityEditor;
using UnityEngine.Analytics;
using Debug = UnityEngine.Debug;

namespace Unity.MeshSync.Editor.Analytics {
    /// <summary>
    /// Factory class to create the required analytics handler for the given MeshSyncAnalyticsData.
    /// </summary>
    internal static class AnalyticsHandlerFactory {
        /// <summary>
        /// Returns the analytics handler depending on which field in MeshSyncAnalyticsData is set.
        /// </summary>
        /// <param name="data">The analytics data container to send</param>
        /// <returns>Handler specific to the data in the container</returns>
        public static AnalyticsDataHandler<MeshSyncAnalyticsData> GetHandler(MeshSyncAnalyticsData data) {
            if (data.sessionStartData.HasValue) {
                return new SessionStartAnalyticsHandler();
            }

            if (data.syncData.HasValue) {
                return new SyncAnalyticsHandler();
            }

            // TODO: If new handlers for MeshSyncAnalyticsData are added, update this to return the new one depending on the field it uses:

            return null;
        }

        public static void RegisterHandlers() {
            // Finds non-abstract subclasses of AnalyticsHandlerFactory and calls Register() on them all:
            var analyticHandlerSubclasses = typeof(AnalyticsHandlerFactory).Assembly.GetTypes().Where(type => !type.IsAbstract && type.IsSubclassOf(typeof(AnalyticsDataHandlerBase)));

            foreach (Type analyticHandlerSubclass in analyticHandlerSubclasses) {
                var t = (AnalyticsDataHandlerBase)Activator.CreateInstance(analyticHandlerSubclass);
                t.Register();
            }
        }
    }

    /// <summary>
    /// Base class for all analytics handlers.
    /// </summary>
    internal abstract class AnalyticsDataHandlerBase {
        internal const string VENDORKEY = "unity.meshsync";

        protected virtual string eventName { get; }
        protected virtual int version { get; }
        protected virtual int eventsPerHour => 10000;

        protected abstract Dictionary<string, DateTime> lastSendTimes { get; }

        protected virtual TimeSpan MinTimeBetweenEvents {
            get { return TimeSpan.FromSeconds(1); }
        }

        bool ShouldSend(string key) {
            // If it is not in the dictionary, it will default to DateTime.MinValue:
            lastSendTimes.TryGetValue(key, out var lastSendTime);

            var now = DateTime.Now;
            var timeSinceLastEvent = now - lastSendTime;
            if (timeSinceLastEvent < MinTimeBetweenEvents) {
                return false;
            }

            lastSendTimes[key] = now;
            return true;
        }

        [Conditional("DEBUG_ANALYTICS")]
        private static void LogIfWarning(AnalyticsResult resp) {
            if (resp != AnalyticsResult.Ok) {
                Debug.LogWarning($"Analytics endpoint reported: {resp} when should be {AnalyticsResult.Ok}");
            }
        }

        /// <summary>
        /// Sends any object to the analytics library.
        /// </summary>
        /// <param name="data">Data to send</param>
        /// <param name="timeKey">Key used to coalesce events</param>
        protected void Send(object data, string timeKey) {
            if (!ShouldSend(timeKey)) {
                return;
            }

            LogIfWarning(
            EditorAnalytics.SendEventWithLimit(
                eventName,
                data,
                version));
        }

        /// <summary>
        /// Registers the handler's event and version with the analytics library.
        /// </summary>
        public void Register() {
            LogIfWarning(
                EditorAnalytics.RegisterEventWithLimit(
                    eventName,
                    eventsPerHour,
                    10,
                    VENDORKEY,
                    version));

        }
    }

    /// <summary>
    /// Base class for analytics handlers that allows specifying the input data type.
    /// </summary>
    internal abstract class AnalyticsDataHandler<T> : AnalyticsDataHandlerBase {
        public abstract void Send(T data);
    }

    /// <summary>
    /// Handles MeshSyncAnalyticsData.sessionStartData
    /// </summary>
    internal class SessionStartAnalyticsHandler : AnalyticsDataHandler<MeshSyncAnalyticsData> {
        private static readonly Dictionary<string, DateTime> lastSendTimesDict = new Dictionary<string, DateTime>();
        protected override Dictionary<string, DateTime> lastSendTimes => lastSendTimesDict;

        struct SessionEventData {
            public string dccToolName;
        }

        protected override string eventName => "meshSync_session_start";
        protected override int version => 2;

        public override void Send(MeshSyncAnalyticsData data) {
            var sessionStartData = data.sessionStartData.Value;

            if (string.IsNullOrEmpty(sessionStartData.DCCToolName)) {
                return;
            }

            var eventData = new SessionEventData {
                dccToolName = sessionStartData.DCCToolName
            };

            Send(eventData, sessionStartData.DCCToolName);
        }
    }

    /// <summary>
    /// Handles MeshSyncAnalyticsData.syncData
    /// </summary>
    internal class SyncAnalyticsHandler : AnalyticsDataHandler<MeshSyncAnalyticsData> {
        private static readonly Dictionary<string, DateTime> lastSendTimesDict = new Dictionary<string, DateTime>();
        protected override Dictionary<string, DateTime> lastSendTimes => lastSendTimesDict;

        struct SyncEventData {
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

            Send(eventData, entityTypeStr + assetTypeStr);
        }
    }

    /// <summary>
    /// Handles installation analytics event.
    /// </summary>
    internal class InstallAnalyticsHandler : AnalyticsDataHandler<string> {
        private static readonly Dictionary<string, DateTime> lastSendTimesDict = new Dictionary<string, DateTime>();
        protected override Dictionary<string, DateTime> lastSendTimes => lastSendTimesDict;

        struct DCCInstallEventData {
            public string meshSyncDccName;
        }

        protected override string eventName => "dccConfigure";
        protected override int version => 4;
        protected override int eventsPerHour => 1000;

        public override void Send(string data) {
            var eventData = new DCCInstallEventData { meshSyncDccName = data.ToLower() };

            Send(eventData, eventData.meshSyncDccName);
        }
    }
}