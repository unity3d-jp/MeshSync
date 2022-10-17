using UnityEditor.ShortcutManagement;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor {

internal static class Shortcuts  {

    [Shortcut(MeshSyncEditorConstants.SHORTCUT_TOGGLE_KEYFRAME, null,  KeyCode.U)]
    static void ToggleFrameMarker(ShortcutArguments args) {
        foreach (Object obj in Selection.objects) {
            FrameMarker marker = obj as FrameMarker;
            if (null == marker) {
                continue;
            }
            FrameMarkerInspector.ToggleMarkerValueByContext(marker);

        }
    }
}

} //end namespace