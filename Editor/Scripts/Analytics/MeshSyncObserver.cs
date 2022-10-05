using System;

using UnityEditor;

namespace Unity.MeshSync.Editor.Analytics {

    /// <summary>
    /// Hacky way to restore observers to MeshSync servers in scene
    /// </summary>
    [InitializeOnLoad]
    internal static class MeshSyncObserverStartUp {

        private static void update() {
            var array = UnityEngine.Object.FindObjectsOfType<BaseMeshSync>(includeInactive: true);

            foreach (var server in array) {
                if (server.getNumObservers == 0) {
                    var observer = new MeshSyncObserver();
                    server.Subscribe(observer);
                }
            }

            if (array.Length > 0 || EditorApplication.timeSinceStartup > 120) {
                EditorApplication.update -= update;
            }
        }

        static MeshSyncObserverStartUp() {
            EditorApplication.update += update;
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