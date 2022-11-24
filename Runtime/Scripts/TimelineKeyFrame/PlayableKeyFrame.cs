using System;
using System.Collections.Generic;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

#if UNITY_EDITOR
using UnityEditor.SceneManagement;
using UnityEditor.Timeline;
#endif

namespace Unity.MeshSync {
    
[Serializable]
internal class PlayableKeyFrame : ISerializationCallbackReceiver {

    internal PlayableKeyFrame(KeyFrameControllerClipData owner) {
        m_clipDataOwner            = owner;
    }

    internal PlayableKeyFrame(KeyFrameControllerClipData owner, PlayableKeyFrame otherKeyFrame) {
        m_clipDataOwner = owner;
        m_enabled       = otherKeyFrame.m_enabled;
        m_localTime     = otherKeyFrame.m_localTime;
        m_playFrame     = otherKeyFrame.m_playFrame;
        m_keyFrameMode  = otherKeyFrame.m_keyFrameMode;
        m_userNote      = otherKeyFrame.m_userNote;
    }       
   
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    public void OnBeforeSerialize() {
        
    }

    public void OnAfterDeserialize() {
        if (null == m_marker)
            return;

        m_marker.SetOwner(this);
    }    
    #endregion //ISerializationCallbackReceiver
    

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal void Destroy() {
        TryDeleteMarker();
    }

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    internal void                       SetOwner(KeyFrameControllerClipData owner) {  m_clipDataOwner = owner;}
    internal KeyFrameControllerClipData GetOwner()                                 {  return m_clipDataOwner; }
    internal double                     GetLocalTime()                             { return m_localTime; }

    internal int GetPlayFrame() { return m_playFrame; }
    
    internal int GetIndex() { return m_index; }
    internal void   SetIndexAndLocalTime(int index, double localTime) {
        m_index     = index; 
        m_localTime = localTime;
    }

    internal void SetPlayFrame(int playFrame) {
        m_playFrame = playFrame;
    }

    internal void SetKeyFrameMode(KeyFrameMode mode) {
        if (m_keyFrameMode == mode) {
            return;
        }
        
        m_keyFrameMode = mode;
#if UNITY_EDITOR
        TimelineEditor.Refresh(RefreshReason.ContentsModified);
#endif
        
    }

    internal KeyFrameMode GetKeyFrameMode()  => m_keyFrameMode;
    
    internal void SetEnabled(bool enabled) { m_enabled = enabled; }
    internal bool IsEnabled() => m_enabled;
    
    internal TimelineClip GetClipOwner() {
        TimelineClip clip = m_clipDataOwner?.GetOwner();
        return clip;
    }

    internal string GetUserNote() {  return m_userNote;}
    internal void SetUserNote(string userNote)   {  m_userNote = userNote; }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal void SaveStateFromMarker() {
        if (null == m_marker) {
            m_enabled = false;
        } else {
            if (null == m_clipDataOwner)
                return;
            
            TimelineClip clipOwner = m_clipDataOwner.GetOwner();
            m_localTime = m_marker.CalculateKeyFrameLocalTime(clipOwner);
        }
    }
    
    internal void RefreshMarker(bool frameMarkerVisibility) {
        if (null == m_clipDataOwner)
            return;
        
        TrackAsset trackAsset = m_clipDataOwner.GetOwner()?.GetParentTrack();
        //Delete Marker first if it's not in the correct track (e.g: after the TimelineClip was moved)
        if (null!= m_marker && m_marker.parent != trackAsset) {
            TryDeleteMarker();
        }

        //Show/Hide the marker
        if (!m_enabled || (!frameMarkerVisibility)) {
            TryDeleteMarker();
        } else if (null == m_marker && null!=trackAsset) {
            CreateMarker();
        }

        if (m_marker) {
            TimelineClip clipOwner = m_clipDataOwner.GetOwner();
            m_marker.Init(this, clipOwner);
        }
    }
    
    internal void RefreshMarkerOwner() {
        if (!m_marker) 
            return;
        
        m_marker.SetOwner(this);
    }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    void CreateMarker() {
        TimelineClip clipOwner = m_clipDataOwner.GetOwner();
        TrackAsset trackAsset = clipOwner?.GetParentTrack();
                       
        Assert.IsNotNull(trackAsset);
        Assert.IsNull(m_marker);
               
        m_marker = trackAsset.CreateMarker<KeyFrameMarker>(m_localTime);
    }

    void TryDeleteMarker() {
        if (null == m_marker)
            return;
        
        //Marker should have parent, but in rare cases, it may return null
        TrackAsset track = m_marker.parent;
        if (null != track) {
            track.DeleteMarker(m_marker);            
        }

        m_marker = null;

    }
    
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] private bool m_enabled = true;
    
    [HideInInspector][SerializeField] private double m_localTime;
    
    [HideInInspector][SerializeField] private int          m_playFrame    = 0; //the frame that should be displayed for this localTime
    [HideInInspector][SerializeField] private KeyFrameMode m_keyFrameMode = KeyFrameMode.Continuous; 

    
    [HideInInspector][SerializeField] private KeyFrameMarker        m_marker = null;
    [HideInInspector][SerializeField] private string                m_userNote;
    [NonSerialized]                   private KeyFrameControllerClipData m_clipDataOwner = null;

    private int m_index;

}

} //end namespace


//[Note-Sin: 2022-11-17] PlayableKeyFrame
//* A structure to store if we should use the image at a particular frame
//* SceneCacheTrack owns PlayableKeyFrame, which is associated with a TimelineClip.
//* PlayableKeyFrame owns KeyFrameMarker.
