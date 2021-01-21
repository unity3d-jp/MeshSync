using System;

using UnityEngine.Playables;


namespace Unity.StreamingImageSequence {

internal abstract class BaseTimelineClipDataPlayableAsset : PlayableAsset{

    protected virtual void OnDestroy() {          
        m_clipData?.Destroy();           
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    internal void BindTimelineClipData(BaseTimelineClipData data) { m_clipData = data;}         
    internal BaseTimelineClipData GetBoundTimelineClipData() { return m_clipData; }    
    
//----------------------------------------------------------------------------------------------------------------------
    
    //[Note-sin: 2020-6-30] BaseTimelineClipData stores extra data for TimelineClip
    [NonSerialized] private BaseTimelineClipData m_clipData = null;
    
}

} //end namespace

