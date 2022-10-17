using System;
using System.Collections.Generic;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

#if UNITY_EDITOR
using UnityEditor.SceneManagement;
#endif

namespace Unity.MeshSync {
    
[Serializable]
internal class SISPlayableFrame : ISerializationCallbackReceiver {

    internal SISPlayableFrame(PlayableFrameClipData owner) {
        m_clipDataOwner = owner;        
        m_boolProperties = new Dictionary<PlayableFramePropertyID, PlayableFrameBoolProperty>();  
    }

    internal SISPlayableFrame(PlayableFrameClipData owner, SISPlayableFrame otherFrame) {
        m_clipDataOwner = owner;
        m_boolProperties = otherFrame.m_boolProperties;
        m_localTime = otherFrame.m_localTime;
    }       
    
    
//----------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    public void OnBeforeSerialize() {
        if (null != m_boolProperties) {
            m_serializedBoolProperties = new List<PlayableFrameBoolProperty>(m_boolProperties.Count);
            foreach (KeyValuePair<PlayableFramePropertyID, PlayableFrameBoolProperty> kv in m_boolProperties) {
                m_serializedBoolProperties.Add(kv.Value);
            }        
            
        } else {
            m_serializedBoolProperties = new List<PlayableFrameBoolProperty>();            
        }
        
    }

    public void OnAfterDeserialize() {
        m_boolProperties = new Dictionary<PlayableFramePropertyID, PlayableFrameBoolProperty>();
        if (null != m_serializedBoolProperties) {
            foreach (PlayableFrameBoolProperty prop in m_serializedBoolProperties) {
                PlayableFramePropertyID id = prop.GetID();
                m_boolProperties[id] = new PlayableFrameBoolProperty(id, prop.GetValue());
            }            
        } 
        
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

    internal int GetIndex() { return m_index; }
    internal void   SetIndexAndLocalTime(int index, double localTime) {
        m_index = index; 
        m_localTime = localTime;        
    }

    internal TimelineClip GetClipOwner() {
        TimelineClip clip = m_clipDataOwner?.GetOwner();
        return clip;
    }

    internal string GetUserNote() {  return m_userNote;}
    internal void SetUserNote(string userNote)   {  m_userNote = userNote; }    
    
//----------------------------------------------------------------------------------------------------------------------
    //Property
    internal bool GetBoolProperty(PlayableFramePropertyID propertyID) {
        if (null!=m_boolProperties && m_boolProperties.ContainsKey(propertyID)) {
            return m_boolProperties[propertyID].GetValue();
        }

        switch (propertyID) {
            case PlayableFramePropertyID.USED: return true;
            case PlayableFramePropertyID.LOCKED: return false;
                default: return false;
        }        
    }
    
    

    internal void SetBoolProperty(PlayableFramePropertyID id, bool val) {
#if UNITY_EDITOR        
        if (GetBoolProperty(id) != val) {
            EditorSceneManager.MarkAllScenesDirty();            
        }
#endif        
        m_boolProperties[id] = new PlayableFrameBoolProperty(id, val);
        
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

    [HideInInspector][SerializeField] private List<PlayableFrameBoolProperty> m_serializedBoolProperties;
    [HideInInspector][SerializeField] private double                          m_localTime;    
    [HideInInspector][SerializeField] private FrameMarker                     m_marker = null;
    [HideInInspector][SerializeField] private string                          m_userNote;
    [NonSerialized]                   private PlayableFrameClipData           m_clipDataOwner = null;

    private int m_index;
    
    private Dictionary<PlayableFramePropertyID, PlayableFrameBoolProperty> m_boolProperties;




}

} //end namespace


//A structure to store if we should use the image at a particular frame