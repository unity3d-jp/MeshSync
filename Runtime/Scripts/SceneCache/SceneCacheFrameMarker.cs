using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

[Serializable]
[CustomStyle("KeyFrameMarker")]
[HideInMenu]
internal class SceneCacheFrameMarker : Marker, INotification {

    
    internal void Init(PlayableKeyFrame controller, double initialTime) {
        m_playableKeyFrameOwner = controller;
        time = initialTime;
    }     
    
//----------------------------------------------------------------------------------------------------------------------    
    
    //return false to indicate that this marker has been invalidated
    internal bool Refresh() {
        TimelineClip clip = m_playableKeyFrameOwner?.GetClipOwner();;
        if (clip == null) {
            return false;
        } 
        
        time = clip.start + m_playableKeyFrameOwner.GetLocalTime();
        
        return true;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    internal void SetOwner(PlayableKeyFrame controller) { m_playableKeyFrameOwner = controller; } 
    internal PlayableKeyFrame GetOwner() { return m_playableKeyFrameOwner; } 
    
//----------------------------------------------------------------------------------------------------------------------    
    public PropertyName id { get; } //use default implementation

    private PlayableKeyFrame m_playableKeyFrameOwner;
       
}

} //end namespace


//A visual representation (Marker) of SceneCachePlayableFrame