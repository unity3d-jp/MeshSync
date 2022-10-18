using System;
using System.Collections.Generic;
using System.Linq;

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

        /// <summary>
        /// Send an event that the user has synched some data through MeshSync
        /// </summary>
        /// <param name="data">Description of data synched</param>
        void UserSyncedData(MeshSyncAnalyticsData data);

        /// <summary>
        /// Whether or not to coalesce events to prevent over reporting
        /// </summary>
        bool CoalesceEvents { get; set; }

        /// <summary>
        /// Minimum time between syncs to analytics backend
        /// </summary>
        TimeSpan MinTimeBetweenSync { get; set; }
    }


    /// <summary>
    /// Simple struct to track when an event was logged
    /// </summary>
    /// <typeparam name="T">Event data type</typeparam>
    internal struct TimedEvent<T>
        where T: struct {

        /// <summary>
        /// Time when event was logged
        /// </summary>
        public DateTimeOffset LogTime;

        /// <summary>
        /// Event data
        /// </summary>
        public T Data;

    }

    internal sealed class MeshSyncAnalytics : IMeshSyncAnalytics {
        private const string VENDORKEY = "unity.meshsync";

        private const string DCCCONFIGURE_INSTALLEDEVENTNAME = "dccConfigure";

        // Data schema version for install event
        private const int DCCCONFIGURE_VERSION = 4;

        private const string MESHSYNC_SYNC_INSTALLEDEVENTNAME = "meshSync_Sync";

        // Data schema version for sync version
        private const int MESHSYNC_SYNC_VERSION = 2;

        private const int DCCCONFIGURE_EVENTS_PER_HOUR = 1000;
        private const int MESHSYNC_SYNC_EVENTS_PER_HOUR = 10000;
        private static readonly TimeSpan DEFAULT_TIME_SPAN = TimeSpan.FromSeconds(10);

        private DateTime lastSync;
        private Stack<TimedEvent<SyncEventData>> syncEventStack;

        public bool CoalesceEvents { get; set; }
        public TimeSpan MinTimeBetweenSync { get; set; }

        private struct DCCInstallEventData {
            public string meshSyncDccName;
        }

        private struct SyncEventData {
            public string assetSyncType;
            public string entitySyncType;
        }


        public MeshSyncAnalytics() {
            this.syncEventStack = new Stack<TimedEvent<SyncEventData>>();
            this.lastSync = DateTime.Now;

            CoalesceEvents = true;
            MinTimeBetweenSync = DEFAULT_TIME_SPAN;
        }

        private static void logIfWarning(AnalyticsResult resp) {
            if (resp != AnalyticsResult.Ok && resp != AnalyticsResult.TooManyRequests) {
                Debug.LogWarning($"Analytics endpoint reported: {resp} when should be {AnalyticsResult.Ok}");
            }
        }

        /// <summary>
        /// Static constructor, executed once per domain reload
        /// </summary>
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


        /// <inheritdoc />
        public void UserInstalledPlugin(string dccTool) {
            var data = new DCCInstallEventData { meshSyncDccName = dccTool.ToLower() };
            logIfWarning(EditorAnalytics.SendEventWithLimit(DCCCONFIGURE_INSTALLEDEVENTNAME, data, DCCCONFIGURE_VERSION));
        }

        /// <inheritdoc />
        public void UserSyncedData(MeshSyncAnalyticsData data) {
            if (data.entityType == EntityType.Unknown &&
                data.assetType == AssetType.Unknown) {
                Debug.LogWarning("Invalid event data sent");
                return;
            }

            var assetTypeStr = data.assetType == AssetType.Unknown
                ? "none"
                : data.assetType.ToString().ToLower();

            var entityTypeStr = data.entityType == EntityType.Unknown
                ? "none"
                : data.entityType.ToString().ToLower();

            var eventData = new SyncEventData {
                assetSyncType = assetTypeStr,
                entitySyncType = entityTypeStr
            };

            this.syncEventStack.Push(new TimedEvent<SyncEventData>
            {
                LogTime = DateTimeOffset.Now,
                Data = eventData
            });

            if (!CoalesceEvents) {
                var evts = this.syncEventStack.Select(evt => evt.Data).ToArray();

                foreach (var evt in evts) {
                    logIfWarning(
                        EditorAnalytics.SendEventWithLimit(
                            MESHSYNC_SYNC_INSTALLEDEVENTNAME,
                            evt,
                            MESHSYNC_SYNC_VERSION));
                }
                this.syncEventStack.Clear();
            }
            else if ((DateTime.Now - this.lastSync) > MinTimeBetweenSync) {

                // Group entity sync events by type
                var syncEntityTypeLookup = this.syncEventStack
                    .Where(x => x.Data.entitySyncType != "none")
                    .ToLookup(x => x.Data.entitySyncType);

                // Group asset sync events by type
                var syncAssetTypeLookup = this.syncEventStack
                    .Where(x => x.Data.assetSyncType != "none")
                    .ToLookup(x => x.Data.assetSyncType);

                // Coalesce events by time
                var syncEntityTypeFinalBatch = CoalesceEventsByTime(syncEntityTypeLookup);
                var syncAssetTypeFinalBatch = CoalesceEventsByTime(syncAssetTypeLookup);

                var combined = new List<TimedEvent<SyncEventData>>();

                combined.AddRange(syncEntityTypeFinalBatch);
                combined.AddRange(syncAssetTypeFinalBatch);

                // Re-order combined events by time
                var sorted = combined
                    .OrderBy(evt => evt.LogTime)
                    .Select(evt => evt.Data);

                foreach (var evt in sorted) {
                    logIfWarning(
                        EditorAnalytics.SendEventWithLimit(
                            MESHSYNC_SYNC_INSTALLEDEVENTNAME,
                            evt,
                            MESHSYNC_SYNC_VERSION));

                }

                this.lastSync = DateTime.Now;

                this.syncEventStack.Clear();

            }
        }

        /// <summary>
        /// Groups up events by time interval and picks one per second
        /// </summary>
        /// <typeparam name="T">Event data type</typeparam>
        /// <param name="events">Events grouped by type</param>
        /// <returns>Flattened list of events limited to one per second per event type</returns>
        private static IEnumerable<TimedEvent<T>> CoalesceEventsByTime<T>(ILookup<string, TimedEvent<T>> events)
            where T: struct {
            var list = new List<TimedEvent<T>>();

            foreach (var group in events) {
                var perSecond = group.GroupBy(evt => evt.LogTime.ToUnixTimeSeconds())
                    .Select(x => x.First());

                list.AddRange(perSecond);
            }

            return list;
        }
    }
}