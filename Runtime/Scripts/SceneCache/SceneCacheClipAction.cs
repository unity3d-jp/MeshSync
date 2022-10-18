using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEditor.Timeline.Actions;

namespace Unity.MeshSync {

[MenuEntry("Add KeyFrame", MenuPriority.MarkerActionSection.start)]
//[Shortcut(Shortcuts.Clip.trimStart), UsedImplicitly]
class AddKeyFrameSceneCacheClipAction : ClipAction {
    public override ActionValidity Validate(IEnumerable<TimelineClip> clips) {

        if (clips.Contains<SceneCachePlayableAsset>()) {
            return ActionValidity.Valid;
        }
        
        return ActionValidity.NotApplicable;
    }

    public override bool Execute(IEnumerable<TimelineClip> clips) {
        return true;
    }
}

[MenuEntry("Delete KeyFrame", MenuPriority.MarkerActionSection.start)]
//[Shortcut(Shortcuts.Clip.trimStart), UsedImplicitly]
class DeleteKeyFrameSceneCacheClipAction : ClipAction {
    public override ActionValidity Validate(IEnumerable<TimelineClip> clips) {
        if (clips.Contains<SceneCachePlayableAsset>()) {
            return ActionValidity.Valid;
        }
        return ActionValidity.NotApplicable;
    }

    public override bool Execute(IEnumerable<TimelineClip> clips) {
        return true;
    }
}


} //end namespace