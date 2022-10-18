using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

[Serializable]
[CustomStyle("KeyFrameMarker")]
[HideInMenu]
internal class FrameMarker : Marker, INotification {

    
    internal void Init(SISPlayableFrame controller, double initialTime) {
        m_playableFrameOwner = controller;
        time = initialTime;
    }     
    
//----------------------------------------------------------------------------------------------------------------------    
    
    //return false to indicate that this marker has been invalidated
    internal bool Refresh() {
        TimelineClip clip = m_playableFrameOwner?.GetClipOwner();;
        if (clip == null) {
            return false;
        } 
        
        time = clip.start + m_playableFrameOwner.GetLocalTime();
        
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    internal void SetOwner(SISPlayableFrame controller) { m_playableFrameOwner = controller; } 
    internal SISPlayableFrame GetOwner() { return m_playableFrameOwner; } 
    
    internal KeyFrameMode GetKeyFrameMode() {
        return (KeyFrameMode) m_playableFrameOwner.GetProperty(KeyFramePropertyID.Mode);

    }
    internal void SetKeyFrameMode(KeyFrameMode mode) { m_playableFrameOwner.SetProperty(KeyFramePropertyID.Mode, (int) mode); }
    
//----------------------------------------------------------------------------------------------------------------------    
    public PropertyName id { get; } //use default implementation

    private SISPlayableFrame m_playableFrameOwner;
       
}

} //end namespace


//A visual representation (Marker) of SISPlayableFrame