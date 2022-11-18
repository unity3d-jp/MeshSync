using System.Collections.Generic;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {
internal static class TimelineClipExtensions {
    
    //[TODO-sin: 2022-11-17] Move to FIU
    internal static bool Contains<T>(this IEnumerable<TimelineClip> clips) where T : PlayableAsset {
        foreach (TimelineClip clip in clips) {
            T asset = clip.asset as T;
            if (null != asset)
                return true;
        }
        return false;
    }
}
} //end namespace