using System;

using UnityEditor;

namespace Unity.MeshSync.Editor.Analytics {

    /// <summary>
    /// Hacky way to restore observers to MeshSync servers in scene
    /// </summary>
    [InitializeOnLoad]
    internal static class MeshSyncObserverStartUp {
        private const string SESSION_STATE_FLAG = "meshSyncAnalyticsSetupFlag";
        private const int TIMEOUT_SECONDS = 120;

        private static void update() {
            var alreadySetup = SessionState.GetBool(SESSION_STATE_FLAG, defaultValue: false);

            // If the setup was already done, we still need to do it again at least once after a domain reload
            if (alreadySetup) {
                EditorApplication.update -= update;
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

            if (success || EditorApplication.timeSinceStartup > TIMEOUT_SECONDS) {
                SessionState.SetBool(SESSION_STATE_FLAG, value: true);
                EditorApplication.update -= update;
            }
        }

        static MeshSyncObserverStartUp() {
            EditorApplication.update += update;
            EditorApplication.playModeStateChanged += (_) => update();
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