using System.Diagnostics;
using Debug = UnityEngine.Debug;

namespace Unity.MeshSync {
    internal static class MeshSyncLogger {
        [Conditional("VERBOSE_LOGS")]
        public static void VerboseLog(string message) {
            Debug.Log($"[MeshSync] {message}");
        }
    }
}
