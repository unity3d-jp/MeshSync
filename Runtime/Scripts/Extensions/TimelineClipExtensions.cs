using System.Collections.Generic;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {
internal static class TimelineClipExtensions {
    internal static bool Contains<T>(this IEnumerable<TimelineClip> clips) where T : PlayableAsset {
        foreach (TimelineClip clip in clips) {
            T sceneCachePlayableAsset = clip.asset as T;
            if (null != sceneCachePlayableAsset)
                return true;
        }
        return false;
    }
}
} //end namespace