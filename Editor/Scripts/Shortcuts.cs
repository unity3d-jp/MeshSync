using UnityEditor.ShortcutManagement;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor {

internal static class Shortcuts  {

    [Shortcut(MeshSyncEditorConstants.SHORTCUT_CHANGE_KEYFRAME_MODE, null,  KeyCode.M, ShortcutModifiers.Alt)]
    static void ChangeKeyFrameMode(ShortcutArguments args) {
        foreach (Object obj in Selection.objects) {
            FrameMarker marker = obj as FrameMarker;
            if (null == marker) {
                continue;
            }
            FrameMarkerInspector.ChangeKeyFrameMode(marker);

        }
    }
}

} //end namespace