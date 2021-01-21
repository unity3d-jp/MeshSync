using System;
using UnityEngine;
using UnityEngine.Timeline;

namespace Unity.FilmInternalUtilities {

[Serializable]
internal abstract class BaseTimelineClipData : ISerializationCallbackReceiver {

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
    
    //The owner of this ClipData
    [NonSerialized] private TimelineClip  m_clipOwner = null;

#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_version = CUR_TIMELINE_CLIP_DATA_VERSION;        
#pragma warning restore 414    

    private const int    CUR_TIMELINE_CLIP_DATA_VERSION = 1;
    
}


} //end namespace


