using System;
using UnityEngine.Timeline;

namespace Unity.StreamingImageSequence {

[Serializable]
internal class SceneCacheClipData : BaseTimelineClipData {

    internal SceneCacheClipData(TimelineClip owner) : base(owner) {
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    private       bool   m_frameMarkersVisibility           = false;
    
}


} //end namespace


