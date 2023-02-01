using System;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {
[Serializable]
[CustomStyle("KeyFrameMarker")]
[HideInMenu]
internal class KeyFrameMarker : Marker, INotification, ICanRefresh {
    internal void Init(PlayableKeyFrame controller, TimelineClip clip) {
        m_playableKeyFrameOwner = controller;
        time                    = CalculateMarkerTime(clip, m_playableKeyFrameOwner.GetLocalTime());
    }

//----------------------------------------------------------------------------------------------------------------------    

    //return false to indicate that this marker has been invalidated
    public bool Refresh() {
        TimelineClip clip = m_playableKeyFrameOwner?.GetClipOwner();
        ;
        if (clip == null) return false;

        time = CalculateMarkerTime(clip, m_playableKeyFrameOwner.GetLocalTime());
        return true;
    }

    private static double CalculateMarkerTime(TimelineClip clip, double keyFrameLocalTime) {
        return clip.start + (-clip.clipIn + keyFrameLocalTime) / clip.timeScale;
    }

    internal double CalculateKeyFrameLocalTime(TimelineClip clip) {
        return (time - clip.start) * clip.timeScale + clip.clipIn;
    }


//----------------------------------------------------------------------------------------------------------------------

    internal void SetOwner(PlayableKeyFrame controller) {
        m_playableKeyFrameOwner = controller;
    }

    internal PlayableKeyFrame GetOwner() {
        return m_playableKeyFrameOwner;
    }

//----------------------------------------------------------------------------------------------------------------------    
    public PropertyName id { get; } //use default implementation

    private PlayableKeyFrame m_playableKeyFrameOwner;
}
} //end namespace


//A visual representation (Marker) of SceneCachePlayableFrame