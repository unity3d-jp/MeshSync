using System;
using UnityEngine.Timeline;
using Object = UnityEngine.Object;
#if UNITY_EDITOR        
using UnityEditor.Timeline;
#endif

namespace Unity.MeshSync {
    
[Serializable]
internal static class PlayableFrameExtension {

    internal static void SetUsed(this SISPlayableFrame playableFrame, bool used) {
#if UNITY_EDITOR        
        bool prevUsed = playableFrame.IsUsed();        
#endif
        
        playableFrame.SetBoolProperty(PlayableFramePropertyID.USED, used);

#if UNITY_EDITOR        
        //Refresh 
        if (used != prevUsed) {
            TimelineEditor.Refresh(RefreshReason.ContentsModified);
        }
#endif        
    }

    internal static bool IsUsed(this SISPlayableFrame playableFrame) {
        return null != playableFrame && playableFrame.GetBoolProperty(PlayableFramePropertyID.USED);
    }
//----------------------------------------------------------------------------------------------------------------------    
    
    internal static void SetLocked(this SISPlayableFrame playableFrame, bool used) {
        playableFrame.SetBoolProperty(PlayableFramePropertyID.LOCKED, used);
    }

    internal static bool IsLocked(this SISPlayableFrame playableFrame) {
        return null != playableFrame && playableFrame.GetBoolProperty(PlayableFramePropertyID.LOCKED);
    }
    
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

