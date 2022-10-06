using System;

using UnityEditor;

namespace Unity.MeshSync.Editor.Analytics {

    /// <summary>
    /// Hacky way to restore observers to MeshSync servers in scene
    /// </summary>
    [InitializeOnLoad]
    internal static class MeshSyncObserverStartUp {
        private const string SESSION_STATE_FLAG = "meshSyncAnalyticsSetupFlag";
        private static DateTime m_check_Start;
        private static readonly TimeSpan INTERVAL = TimeSpan.FromSeconds(3);

        private static void updateCallback(bool limitChecks) {
            // Limit checks to once every 3 seconds
            if ((DateTime.Now - m_check_Start < INTERVAL) && limitChecks) {
                return;
            }

            m_check_Start = DateTime.Now;

            var alreadySetup = SessionState.GetBool(SESSION_STATE_FLAG, defaultValue: false);

            // If the setup was already done, we still need to do it again at least once after a domain reload
            if (alreadySetup) {
                EditorApplication.update -= checkWithTimeLimits;
            }

            var array = UnityEngine.Object.FindObjectsOfType<BaseMeshSync>(includeInactive: true);
            bool success = false;

            foreach (var server in array) {
                if (server.getNumObservers == 0) {
                    var observer = new MeshSyncObserver();
                    server.Subscribe(observer);
                    success = true;
                }
            }

            if (success) {
                SessionState.SetBool(SESSION_STATE_FLAG, value: true);
                EditorApplication.update -= checkWithTimeLimits;
            }
        }

        static MeshSyncObserverStartUp() {
            m_check_Start = DateTime.Now;

            EditorApplication.update -= checkWithTimeLimits;
            EditorApplication.update += checkWithTimeLimits;

            EditorApplication.playModeStateChanged -= playModeChangedCallback;
            EditorApplication.playModeStateChanged += playModeChangedCallback;
        }

        private static void playModeChangedCallback(PlayModeStateChange state) {
            if (state == PlayModeStateChange.ExitingPlayMode) {
                updateCallback(limitChecks: false);
            }
        }

        private static void checkWithTimeLimits() {
            updateCallback(limitChecks: true);
        }
    }

    /// <summary>
    /// Observer that reports analytics events
    /// </summary>
    internal sealed class MeshSyncObserver : IObserver<MeshSyncAnalyticsData> {
        private readonly IMeshSyncAnalytics analytics;

        public MeshSyncObserver() {
            analytics = MeshSyncAnalyticsFactory.CreateAnalytics();
        }

        public void OnCompleted() {
            throw new NotImplementedException();
        }

        public void OnError(Exception error) {
            throw new NotImplementedException();
        }

        public void OnNext(MeshSyncAnalyticsData value) {
            analytics.UserSyncedData(value);
        }
    }
}