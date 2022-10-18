﻿using System;
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
internal class SISPlayableFrame : ISerializationCallbackReceiver {

    internal SISPlayableFrame(PlayableFrameClipData owner) {
        m_clipDataOwner            = owner;
        m_serializedProperties = new SerializedDictionary<KeyFramePropertyID, PlayableFrameProperty<int>>();
    }

    internal SISPlayableFrame(PlayableFrameClipData owner, SISPlayableFrame otherFrame) {
        m_clipDataOwner        = owner;
        m_serializedProperties = otherFrame.m_serializedProperties;
        m_localTime            = otherFrame.m_localTime;
        m_normalizedAnimationTime = otherFrame.m_normalizedAnimationTime;
    }       
    
    
//----------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    public void OnBeforeSerialize() {
        
    }

    public void OnAfterDeserialize() {
        if (null == m_marker)
            return;

        m_marker.SetOwner(this);
    }    
    #endregion //ISerializationCallbackReceiver
    

//----------------------------------------------------------------------------------------------------------------------

    internal void Destroy() {
        if (null == m_marker)
            return;

        DeleteMarker();
    }

//----------------------------------------------------------------------------------------------------------------------
    internal void        SetOwner(PlayableFrameClipData owner) {  m_clipDataOwner = owner;}
    internal PlayableFrameClipData GetOwner()                             {  return m_clipDataOwner; }    
    internal double      GetLocalTime()                                   { return m_localTime; }

    internal double GetNormalizedAnimationTime() { return m_normalizedAnimationTime; }
    
    internal int GetIndex() { return m_index; }
    internal void   SetIndexAndLocalTime(int index, double localTime) {
        m_index                   = index; 
        m_localTime               = localTime;
    }

    internal void SetNormalizedAnimationTime(double animationTime) {
        m_normalizedAnimationTime = animationTime;
    }
    
    internal TimelineClip GetClipOwner() {
        TimelineClip clip = m_clipDataOwner?.GetOwner();
        return clip;
    }

    internal string GetUserNote() {  return m_userNote;}
    internal void SetUserNote(string userNote)   {  m_userNote = userNote; }    
    
//----------------------------------------------------------------------------------------------------------------------
    //Property
    internal int GetProperty(KeyFramePropertyID propertyID) {
        if (null!=m_serializedProperties && m_serializedProperties.TryGetValue(propertyID, out PlayableFrameProperty<int> prop)) {
            return prop.GetValue();
        }

        switch (propertyID) {
            case KeyFramePropertyID.Mode: return (int) KeyFrameMode.Smooth;
            default: return 0;
        }        
    }
    
    

    internal void SetProperty(KeyFramePropertyID id, int val) {
#if UNITY_EDITOR        
        if (GetProperty(id) != val) {
            EditorSceneManager.MarkAllScenesDirty();            
        }
        int prevValue = GetProperty(id);
#endif        
        m_serializedProperties[id] = new PlayableFrameProperty<int>(id, val);
        
#if UNITY_EDITOR
        //Refresh 
        if (val != prevValue) {
            TimelineEditor.Refresh(RefreshReason.ContentsModified);
        }
#endif
        
        
    }
    
    
//----------------------------------------------------------------------------------------------------------------------
    internal void Refresh(bool frameMarkerVisibility) {
        TrackAsset trackAsset = m_clipDataOwner.GetOwner()?.GetParentTrack();
        //Delete Marker first if it's not in the correct track (e.g: after the TimelineClip was moved)
        if (null!= m_marker && m_marker.parent != trackAsset) {
            DeleteMarker();
        }

        //Show/Hide the marker
        if (null != m_marker && !frameMarkerVisibility) {
            DeleteMarker();
        } else if (null == m_marker && null!=trackAsset && frameMarkerVisibility) {
            CreateMarker();
        }

        if (m_marker) {
            TimelineClip clipOwner = m_clipDataOwner.GetOwner();
            m_marker.Init(this, clipOwner.start + m_localTime);
        }
    }
//----------------------------------------------------------------------------------------------------------------------

    void CreateMarker() {
        TimelineClip clipOwner = m_clipDataOwner.GetOwner();
        TrackAsset trackAsset = clipOwner?.GetParentTrack();
                       
        Assert.IsNotNull(trackAsset);
        Assert.IsNull(m_marker);
               
        m_marker = trackAsset.CreateMarker<FrameMarker>(m_localTime);
    }

    void DeleteMarker() {
        Assert.IsNotNull(m_marker);
        
        //Marker should have parent, but in rare cases, it may return null
        TrackAsset track = m_marker.parent;
        if (null != track) {
            track.DeleteMarker(m_marker);            
        }

        m_marker = null;

    }
    
    
//----------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] private SerializedDictionary<KeyFramePropertyID, PlayableFrameProperty<int>> m_serializedProperties;
    
    [HideInInspector][SerializeField] private double m_localTime;
    [HideInInspector][SerializeField] private double m_normalizedAnimationTime = 0;
    
    [HideInInspector][SerializeField] private FrameMarker           m_marker = null;
    [HideInInspector][SerializeField] private string                m_userNote;
    [NonSerialized]                   private PlayableFrameClipData m_clipDataOwner = null;

    private int m_index;

}

} //end namespace


//A structure to store if we should use the image at a particular frame