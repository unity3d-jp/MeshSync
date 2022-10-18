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
                
        playableFrame.SetProperty(PlayableFramePropertyID.USED, used ? 1 : 0);

#if UNITY_EDITOR        
        //Refresh 
        if (used != prevUsed) {
            TimelineEditor.Refresh(RefreshReason.ContentsModified);
        }
#endif        
    }

    internal static bool IsUsed(this SISPlayableFrame playableFrame) {
        return true;
    }
//----------------------------------------------------------------------------------------------------------------------    
    
    internal static void SetKeyFrameMode(this SISPlayableFrame playableFrame, KeyFrameMode mode) {
        playableFrame.SetProperty(PlayableFramePropertyID.LOCKED, (int) mode);
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

