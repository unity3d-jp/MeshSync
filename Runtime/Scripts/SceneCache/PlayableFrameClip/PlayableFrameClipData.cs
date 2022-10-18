using System;
using System.Collections.Generic;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Serialization;
using UnityEngine.Timeline;

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync {

[Serializable]
internal abstract class PlayableFrameClipData : BaseClipData {

    protected PlayableFrameClipData() {
        m_playableFrames = new List<SISPlayableFrame>();
    }

    protected PlayableFrameClipData(TimelineClip clipOwner) {
        SetOwner(clipOwner);
        m_playableFrames = new List<SISPlayableFrame>();
    }

    protected PlayableFrameClipData(TimelineClip owner, PlayableFrameClipData other) : this(owner){
        Assert.IsNotNull(m_playableFrames);
        
        foreach (SISPlayableFrame otherFrame in other.m_playableFrames) {
            SISPlayableFrame newFrame = new SISPlayableFrame(this, otherFrame);
            m_playableFrames.Add(newFrame);
        }
        
        m_frameMarkersRequested = other.m_frameMarkersRequested;
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    protected override void OnBeforeSerializeInternalV() {
    }

    protected override void OnAfterDeserializeInternalV() {
        foreach (SISPlayableFrame playableFrame in m_playableFrames) {
            playableFrame.SetOwner(this);
        }
    }    
    #endregion
//----------------------------------------------------------------------------------------------------------------------
    internal override void DestroyV() {

        foreach (SISPlayableFrame playableFrame in m_playableFrames) {
            playableFrame.Destroy();
        }

    }
    

//----------------------------------------------------------------------------------------------------------------------

    internal bool AreFrameMarkersRequested() {
        return m_frameMarkersRequested;
    }

    internal void RequestFrameMarkers(bool req, bool forceShow = false) {

        if (req == m_frameMarkersRequested)
            return;
        
#if UNITY_EDITOR
        Undo.RegisterCompleteObjectUndo(GetOwner().GetParentTrack(),"StreamingImageSequence Show/Hide FrameMarker");
        m_forceShowFrameMarkers = forceShow && req;
#endif        
        m_frameMarkersRequested = req;
        if (UpdateFrameMarkersVisibility()) {
            RefreshPlayableFrames();                    
        }
    }


    internal int GetNumPlayableFrames() { return m_playableFrames.Count;}

#if UNITY_EDITOR
    internal void SetInspectedProperty(KeyFramePropertyID id) { m_inspectedPropertyID = id; }

    internal KeyFramePropertyID GetInspectedProperty() { return m_inspectedPropertyID; }

    internal void UpdateTimelineWidthPerFrame(float visibleRectWidth, double visibleTime, double fps, double timeScale) {
        int numFrames = Mathf.RoundToInt((float)
            ((visibleTime) * fps / timeScale) 
        );
                
        double widthPerFrame = visibleRectWidth / numFrames;
        m_timelineWidthPerFrame = widthPerFrame;
        if (UpdateFrameMarkersVisibility()) {
            RefreshPlayableFrames();
            
        }                
    }
    
#endif    
    
//----------------------------------------------------------------------------------------------------------------------    
    private static SISPlayableFrame CreatePlayableFrame(PlayableFrameClipData owner, int index, double timePerFrame) 
    {
        SISPlayableFrame playableFrame = new SISPlayableFrame(owner);
        playableFrame.SetIndexAndLocalTime(index, timePerFrame * index);
        return playableFrame;
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    internal void ResetPlayableFrames() {
        DestroyPlayableFrames();

        //Recalculate the number of frames and create the marker's ground truth data
        int numFrames = TimelineUtility.CalculateNumFrames(GetOwner());
        m_playableFrames = new List<SISPlayableFrame>(numFrames);
        UpdatePlayableFramesSize(numFrames);                
    }

//----------------------------------------------------------------------------------------------------------------------
    internal void SetAllPlayableFramesProperty(KeyFramePropertyID id, bool val) {
        foreach (SISPlayableFrame playableFrame in m_playableFrames) {
            playableFrame.SetProperty(id, val ? 1 : 0);
        }
    }

//----------------------------------------------------------------------------------------------------------------------
    private void DestroyPlayableFrames() {
        if (null == m_playableFrames)
            return;
        
        foreach (SISPlayableFrame frame in m_playableFrames) {
            frame?.Destroy();
        }        
    } 
    
//----------------------------------------------------------------------------------------------------------------------
    
    //Resize PlayableFrames and used the previous values
    internal void RefreshPlayableFrames() {

        TimelineClip clipOwner = GetOwner(); 
            
        
        //Clip doesn't have parent. Might be because the clip is being moved 
        if (null == clipOwner.GetParentTrack()) {
            return;
        }        
        
        int numIdealNumPlayableFrames = TimelineUtility.CalculateNumFrames(clipOwner);
      
        //Change the size of m_playableFrames and reinitialize if necessary
        int prevNumPlayableFrames = m_playableFrames.Count;
        if (numIdealNumPlayableFrames != prevNumPlayableFrames) {
            
            //Change the size of m_playableFrames and reinitialize if necessary
            List<KeyFrameMode> prevKeyFrameModes = new List<KeyFrameMode>(prevNumPlayableFrames);
            foreach (SISPlayableFrame frame in m_playableFrames) {
                //if frame ==null, just use the default
                prevKeyFrameModes.Add(null == frame ? KeyFrameMode.Smooth : (KeyFrameMode) frame.GetProperty(KeyFramePropertyID.Mode)); 
            }

            UpdatePlayableFramesSize(numIdealNumPlayableFrames);
            
            //Reinitialize 
            if (prevNumPlayableFrames > 0) {
                int minNumPlayableFrames = Math.Min(prevNumPlayableFrames, m_playableFrames.Count);
                for (int i = 0; i < minNumPlayableFrames; ++i) {
                    m_playableFrames[i].SetProperty(KeyFramePropertyID.Mode, (int)prevKeyFrameModes[i]);
                }
            }
        }
        
        //Refresh all markers
        double timePerFrame           = TimelineUtility.CalculateTimePerFrame(clipOwner);                
        int    numPlayableFrames      = m_playableFrames.Count;
        for (int i = 0; i < numPlayableFrames; ++i) {
            m_playableFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_playableFrames[i].Refresh(m_frameMarkersVisibility);
        }
        
    }        
    
    
//----------------------------------------------------------------------------------------------------------------------
    //may return null
    internal SISPlayableFrame GetPlayableFrame(int index) {
        if (null == m_playableFrames || index >= m_playableFrames.Count)
            return null;
        
        Assert.IsTrue(null!=m_playableFrames && index < m_playableFrames.Count);
        return m_playableFrames[index];
    }


//----------------------------------------------------------------------------------------------------------------------
    
    private void UpdatePlayableFramesSize(int reqPlayableFramesSize) {
        TimelineClip clipOwner = GetOwner();
        Assert.IsNotNull(clipOwner);

        double timePerFrame = TimelineUtility.CalculateTimePerFrame(clipOwner);
        //Resize m_playableFrames
        if (m_playableFrames.Count < reqPlayableFramesSize) {
            int             numNewPlayableFrames = (reqPlayableFramesSize - m_playableFrames.Count);
            List<SISPlayableFrame> newPlayableFrames = new List<SISPlayableFrame>(numNewPlayableFrames);           
            for (int i = m_playableFrames.Count; i < reqPlayableFramesSize; ++i) {
                newPlayableFrames.Add(CreatePlayableFrame(this,i, timePerFrame));
            }            
            m_playableFrames.AddRange(newPlayableFrames);                
        }

        if (m_playableFrames.Count > reqPlayableFramesSize) {
            int numLastPlayableFrames = m_playableFrames.Count;
            for (int i = reqPlayableFramesSize; i < numLastPlayableFrames; ++i) {
                SISPlayableFrame curFrame = m_playableFrames[i];
                curFrame?.Destroy();
            }            
            m_playableFrames.RemoveRange(reqPlayableFramesSize, numLastPlayableFrames - reqPlayableFramesSize);
        }
            
        Assert.IsTrue(m_playableFrames.Count == reqPlayableFramesSize);
           
        for (int i = 0; i < reqPlayableFramesSize; ++i) {
            SISPlayableFrame curPlayableFrame = m_playableFrames[i];
            Assert.IsNotNull(curPlayableFrame);                
            m_playableFrames[i].SetIndexAndLocalTime(i, timePerFrame * i);
            
        }                        
    }

//----------------------------------------------------------------------------------------------------------------------

    //return true if the visibility has changed
    private bool UpdateFrameMarkersVisibility() {
        
        bool prevVisibility = m_frameMarkersVisibility;
#if UNITY_EDITOR        
        const int FRAME_MARKER_WIDTH_THRESHOLD = 10; //less: tend to keep drawing markers
        m_frameMarkersVisibility = m_frameMarkersRequested && 
            (m_forceShowFrameMarkers || m_timelineWidthPerFrame > FRAME_MARKER_WIDTH_THRESHOLD);
#else
        m_frameMarkersVisibility = m_frameMarkersRequested;
#endif
        return prevVisibility != m_frameMarkersVisibility;
    }
       
//----------------------------------------------------------------------------------------------------------------------    
    
    //The ground truth for using/dropping an image in a particular frame. See the notes below
    [SerializeField] private List<SISPlayableFrame> m_playableFrames;
    [SerializeField] [HideInInspector] private bool m_frameMarkersRequested = true;

#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_playableFrameClipDataVersion = CUR_PLAYABLE_FRAME_CLIP_DATA_VERSION;        
#pragma warning restore 414    
    
#if UNITY_EDITOR    
    private KeyFramePropertyID m_inspectedPropertyID   = KeyFramePropertyID.Mode;
    private double                  m_timelineWidthPerFrame = Int16.MaxValue;
    private bool                    m_forceShowFrameMarkers = false;
#endif

    private       bool   m_frameMarkersVisibility           = true;
    
    private const int    CUR_PLAYABLE_FRAME_CLIP_DATA_VERSION = 1;
    
}


} //end namespace


//[Note-Sin: 2020-7-15] SISPlayableFrame
//StreamingImageSequenceTrack owns SISPlayableFrame, which is associated with a TimelineClip.
//SISPlayableFrame is a ScriptableObject and owns FrameMarker.
