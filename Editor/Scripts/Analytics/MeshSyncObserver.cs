using System;
using UnityEditor;

namespace Unity.MeshSync.Editor.Analytics {
/// <summary>
/// Hacky way to restore observers to MeshSync servers in scene
/// </summary>
[InitializeOnLoad]
internal static class MeshSyncObserverStartUp {
    private const           string   SESSION_STATE_FLAG = "meshSyncAnalyticsSetupFlag";
    private static          DateTime m_check_Start;
    private static readonly TimeSpan INTERVAL = TimeSpan.FromSeconds(3);

    private static void updateCallback(bool limitChecks) {
        // Limit checks to once every 3 seconds
        if (DateTime.Now - m_check_Start < INTERVAL && limitChecks) return;

        m_check_Start = DateTime.Now;

        bool alreadySetup = SessionState.GetBool(SESSION_STATE_FLAG, false);

        // If the setup was already done, we still need to do it again at least once after a domain reload
        if (alreadySetup) EditorApplication.update -= checkWithTimeLimits;

        BaseMeshSync[] array   = UnityEngine.Object.FindObjectsOfType<BaseMeshSync>(true);
        bool           success = false;

        foreach (BaseMeshSync server in array)
            if (server.getNumObservers == 0) {
                MeshSyncObserver observer = new MeshSyncObserver();
                server.Subscribe(observer);
                success = true;
            }

        if (success) {
            SessionState.SetBool(SESSION_STATE_FLAG, true);
            EditorApplication.update -= checkWithTimeLimits;
        }
    }

    static MeshSyncObserverStartUp() {
        m_check_Start = DateTime.Now;

        EditorApplication.update -= checkWithTimeLimits;
        EditorApplication.update += checkWithTimeLimits;

        EditorApplication.playModeStateChanged -= playModeChangedCallback;
        EditorApplication.playModeStateChanged += playModeChangedCallback;

        EditorApplication.hierarchyChanged -= checkWithoutTimeLimits;
        EditorApplication.hierarchyChanged += checkWithoutTimeLimits;
    }

    private static void playModeChangedCallback(PlayModeStateChange state) {
        if (state == PlayModeStateChange.EnteredEditMode) updateCallback(false);
    }

    private static void checkWithoutTimeLimits() {
        updateCallback(false);
    }

    private static void checkWithTimeLimits() {
        updateCallback(true);
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