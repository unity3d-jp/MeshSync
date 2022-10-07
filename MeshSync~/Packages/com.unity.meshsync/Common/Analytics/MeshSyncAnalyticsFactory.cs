namespace Unity.MeshSync.Common.Analytics {
public static class MeshSyncAnalyticsFactory {

    public static IMeshSyncAnalytics CreateAnalytics() {
#if UNITY_EDITOR
        return new MeshSyncEditorAnalytics();
#else
#endif
        return new MeshSyncRuntimeAnalytics();
    }
}
}