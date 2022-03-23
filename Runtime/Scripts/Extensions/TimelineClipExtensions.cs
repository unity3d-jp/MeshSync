using JetBrains.Annotations;
using Unity.FilmInternalUtilities;
using UnityEngine.Timeline;

namespace Unity.MeshSync {
internal static class TimelineClipExtensions {

    //[TODO-sin: 2022-3-18] Move to FIU
    [CanBeNull]
    internal static T GetClipData<T>(this TimelineClip clip) where T: BaseClipData {
        
        BaseExtendedClipPlayableAsset<T> clipAsset = clip.asset as BaseExtendedClipPlayableAsset<T>;
        if (null == clipAsset)
            return null;
        
        T clipData = clipAsset.GetBoundClipData();
        return clipData;
    }
    
}

} //end namespace