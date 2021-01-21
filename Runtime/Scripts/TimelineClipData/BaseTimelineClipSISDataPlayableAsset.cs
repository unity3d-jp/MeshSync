using System;

using UnityEngine.Playables;


namespace Unity.StreamingImageSequence {

internal abstract class BaseTimelineClipSISDataPlayableAsset : PlayableAsset{

    protected virtual void OnDestroy() {          
        m_timelineClipSISData?.Destroy();           
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    //These methods are necessary "hacks" for knowing the PlayableFrames/FrameMarkers that belong to this
    //this StreamingImageSequencePlayableAssets        
    internal void BindTimelineClipSISData(TimelineClipSISData sisData) { m_timelineClipSISData = sisData;}         
    internal TimelineClipSISData GetBoundTimelineClipSISData() { return m_timelineClipSISData; }
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    //[Note-sin: 2020-6-30] TimelineClipSISData stores extra data of TimelineClip, because we can't extend
    //TimelineClip at the moment. Ideally, it should not be a property of StreamingImageSequencePlayableAsset, because
    //StreamingImageSequencePlayableAsset is an asset, and should be able to be bound to 2 different TimelineClipsSISData.
    //However, for FrameMarker to work, we need to know which TimelineClipSISData is bound to the
    //StreamingImageSequencePlayableAsset, because Marker is originally designed to be owned by TrackAsset, but not
    //TimelineClip        
    [NonSerialized] private TimelineClipSISData m_timelineClipSISData = null;
    
}

} //end namespace

