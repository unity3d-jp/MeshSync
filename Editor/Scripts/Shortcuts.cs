using JetBrains.Annotations;
using UnityEditor.ShortcutManagement;
using UnityEngine;
using UnityEditor;
using UnityEditor.Timeline.Actions;

namespace Unity.MeshSync.Editor {
internal static class Shortcuts  {
    [Shortcut(MeshSyncEditorConstants.SHORTCUT_CHANGE_KEYFRAME_MODE, null,  KeyCode.Tab, ShortcutModifiers.Shift)]
    private static void ChangeKeyFrameMode(ShortcutArguments args) {
        foreach (Object obj in Selection.objects) {
            KeyFrameMarker marker = obj as KeyFrameMarker;
            if (null == marker) continue;
            KeyFrameMarkerInspector.ChangeKeyFrameMode(marker);
        }
    }

    [TimelineShortcut(MeshSyncEditorConstants.SHORTCUT_ADD_KEYFRAME, KeyCode.E, ShortcutModifiers.Shift)]
    [UsedImplicitly]
    public static void HandleShortCut(ShortcutArguments args) {
        Invoker.InvokeWithSelectedClips<AddKeyFrameSceneCacheClipAction>();
    }
}
} //end namespace