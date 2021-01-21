using System;

using UnityEngine.Playables;


namespace Unity.FilmInternalUtilities {

internal abstract class BaseTimelineClipDataPlayableAsset : PlayableAsset{

    protected virtual void OnDestroy() {          
        m_clipData?.Destroy();           
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    internal void BindClipData(BaseClipData data) { m_clipData = data;}         
    internal BaseClipData GetBoundClipData() { return m_clipData; }    
    
//----------------------------------------------------------------------------------------------------------------------
    
    //[Note-sin: 2021-1-21] BaseClipData stores extra data for TimelineClip
    [NonSerialized] private BaseClipData m_clipData = null;
    
}

} //end namespace

