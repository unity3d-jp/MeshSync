using System;

using UnityEngine.Playables;


namespace Unity.StreamingImageSequence {

internal abstract class BaseTimelineClipDataPlayableAsset : PlayableAsset{

    protected virtual void OnDestroy() {          
        m_sceneCacheClipData?.Destroy();           
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    internal void BindTimelineClipSISData(SceneCacheClipData data) { m_sceneCacheClipData = data;}         
    internal SceneCacheClipData GetBoundTimelineClipSISData() { return m_sceneCacheClipData; }    
    
//----------------------------------------------------------------------------------------------------------------------
    
    //[Note-sin: 2020-6-30] SceneCacheClipData stores extra data of TimelineClip
    [NonSerialized] private SceneCacheClipData m_sceneCacheClipData = null;
    
}

} //end namespace

