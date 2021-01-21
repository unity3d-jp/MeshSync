using System;

using UnityEngine.Playables;


namespace Unity.StreamingImageSequence {

internal abstract class BaseTimelineClipDataPlayableAsset : PlayableAsset{

    protected virtual void OnDestroy() {          
        m_timelineClipData?.Destroy();           
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    //These methods are necessary "hacks" for knowing the PlayableFrames/FrameMarkers that belong to this
    //this StreamingImageSequencePlayableAssets        
    internal void BindTimelineClipSISData(TimelineClipData data) { m_timelineClipData = data;}         
    internal TimelineClipData GetBoundTimelineClipSISData() { return m_timelineClipData; }    
    
//----------------------------------------------------------------------------------------------------------------------
    
    //[Note-sin: 2020-6-30] TimelineClipData stores extra data of TimelineClip
    [NonSerialized] private TimelineClipData m_timelineClipData = null;
    
}

} //end namespace

