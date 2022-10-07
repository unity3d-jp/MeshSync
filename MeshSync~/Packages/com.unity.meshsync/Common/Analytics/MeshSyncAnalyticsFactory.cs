using System.Runtime.CompilerServices;

namespace Unity.MeshSync.Common.Analytics {

internal static class MeshSyncAnalyticsFactory {

    internal static IMeshSyncAnalytics CreateAnalytics() {
#if UNITY_EDITOR
        return new MeshSyncEditorAnalytics();
#else
#endif
        return new MeshSyncRuntimeAnalytics();
    }
}
}