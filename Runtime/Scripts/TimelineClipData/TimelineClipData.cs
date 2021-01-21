using System;
using UnityEngine;
using UnityEngine.Timeline;

namespace Unity.StreamingImageSequence {

[Serializable]
internal class TimelineClipData : ISerializationCallbackReceiver {

    internal TimelineClipData(TimelineClip owner) {
        m_clipOwner = owner;
    }

    internal TimelineClipData(TimelineClip owner, TimelineClipData other) : this(owner){        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    public void OnBeforeSerialize() {
    }

    public void OnAfterDeserialize() {
    }    
    #endregion
//----------------------------------------------------------------------------------------------------------------------
    internal void Destroy() {


    }
    

//----------------------------------------------------------------------------------------------------------------------
    internal void SetOwner(TimelineClip clip) { m_clipOwner = clip;}
    
    internal TimelineClip GetOwner() { return m_clipOwner; }

//----------------------------------------------------------------------------------------------------------------------    
    
    //The ground truth for using/dropping an image in a particular frame. See the notes below
    [NonSerialized] private TimelineClip  m_clipOwner = null;

#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_version = CUR_TIMELINE_CLIP_SIS_DATA_VERSION;        
#pragma warning restore 414    
    

    private       bool   m_frameMarkersVisibility           = false;
    
    private const int    CUR_TIMELINE_CLIP_SIS_DATA_VERSION = 1;
    
}


} //end namespace


