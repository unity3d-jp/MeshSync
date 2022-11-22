using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEditor.Timeline;
using UnityEditor.Timeline.Actions;
using UnityEngine.Playables;

namespace Unity.MeshSync.Editor {

[MenuEntry("Add KeyFrame", MenuPriority.MarkerActionSection.start)]
class AddKeyFrameSceneCacheClipAction : ClipAction {
    public override ActionValidity Validate(IEnumerable<TimelineClip> clips) {
        if (clips.Contains<SceneCachePlayableAsset>()) {
            PlayableDirector director = TimelineEditor.inspectedDirector;
            return null == director ? ActionValidity.Invalid : ActionValidity.Valid;
        }
        
        return ActionValidity.NotApplicable;
    }

    public override bool Execute(IEnumerable<TimelineClip> clips) {
        foreach (var clip in clips) {
            SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
            if (null == sceneCachePlayableAsset)
                continue;

            PlayableDirector director = TimelineEditor.inspectedDirector;
            if (null == director)
                return false;
            
            sceneCachePlayableAsset.AddKeyFrame(director.time);
        }
        return true;
    }
}

} //end namespace