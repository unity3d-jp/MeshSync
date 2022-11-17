using System;
using UnityEngine.Timeline;
using Object = UnityEngine.Object;

namespace Unity.MeshSync {
[Serializable]
internal static class SceneCachePlayableFrameExtensions {

    internal static T GetTimelineClipAsset<T>(this PlayableKeyFrame playableKeyFrame) where T : Object {
        TimelineClip timelineClip = playableKeyFrame?.GetOwner().GetOwner();
        if (null == timelineClip)
            return null;

        T clipAsset = timelineClip.asset as T;
        return clipAsset;
    }

}
} //end namespace