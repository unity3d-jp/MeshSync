using System;
using UnityEngine.Timeline;
using Object = UnityEngine.Object;
#if UNITY_EDITOR        
using UnityEditor.Timeline;
#endif

namespace Unity.MeshSync {
    
[Serializable]
internal static class PlayableFrameExtension {

//----------------------------------------------------------------------------------------------------------------------        
    
    internal static T GetTimelineClipAsset<T>(this SISPlayableFrame playableFrame) where T : Object {
        
        TimelineClip     timelineClip  = playableFrame?.GetOwner().GetOwner();
        if (null == timelineClip)
            return null;
        
        T clipAsset = timelineClip.asset as T;
        return clipAsset;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
}

} //end namespace

