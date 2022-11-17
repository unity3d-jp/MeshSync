using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

[Serializable]
[CustomStyle("KeyFrameMarker")]
[HideInMenu]
internal class SceneCacheFrameMarker : Marker, INotification {

    
    internal void Init(SceneCachePlayableFrame controller, double initialTime) {
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
    
    internal void SetOwner(SceneCachePlayableFrame controller) { m_playableFrameOwner = controller; } 
    internal SceneCachePlayableFrame GetOwner() { return m_playableFrameOwner; } 
    
//----------------------------------------------------------------------------------------------------------------------    
    public PropertyName id { get; } //use default implementation

    private SceneCachePlayableFrame m_playableFrameOwner;
       
}

} //end namespace


//A visual representation (Marker) of SceneCachePlayableFrame