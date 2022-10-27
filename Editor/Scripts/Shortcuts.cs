using JetBrains.Annotations;
using UnityEditor.ShortcutManagement;
using UnityEngine;
using UnityEditor;
using UnityEditor.Timeline.Actions;

namespace Unity.MeshSync.Editor {

internal static class Shortcuts  {

    [Shortcut(MeshSyncEditorConstants.SHORTCUT_CHANGE_KEYFRAME_MODE, null,  KeyCode.M, ShortcutModifiers.Shift)]
    static void ChangeKeyFrameMode(ShortcutArguments args) {
        foreach (Object obj in Selection.objects) {
            FrameMarker marker = obj as FrameMarker;
            if (null == marker) {
                continue;
            }
            FrameMarkerInspector.ChangeKeyFrameMode(marker);

        }
    }
    
    [TimelineShortcut(MeshSyncEditorConstants.SHORTCUT_ADD_KEYFRAME, KeyCode.A, ShortcutModifiers.Shift), UsedImplicitly]
    public static void HandleShortCut(ShortcutArguments args) {
        Invoker.InvokeWithSelectedClips<AddKeyFrameSceneCacheClipAction>();
    }    
}

} //end namespace