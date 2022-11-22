using System;
using System.Collections.Generic;
using JetBrains.Annotations;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.Timeline;
#endif

namespace Unity.MeshSync {

[Serializable]
internal abstract class KeyFrameControllerClipData : BaseClipData {

    protected KeyFrameControllerClipData() {
        m_keyFrames = new List<PlayableKeyFrame>();
    }

    protected KeyFrameControllerClipData(TimelineClip owner, KeyFrameControllerClipData other) {
        SetOwner(owner);
        m_keyFrames = new List<PlayableKeyFrame>();
        
        foreach (PlayableKeyFrame otherFrame in other.m_keyFrames) {
            PlayableKeyFrame newKeyFrame = new PlayableKeyFrame(this, otherFrame);
            m_keyFrames.Add(newKeyFrame);
        }
        
        m_keyFrameMarkersRequested = other.m_keyFrameMarkersRequested;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    protected override void OnBeforeSerializeInternalV() {
    }

    protected override void OnAfterDeserializeInternalV() {
        foreach (PlayableKeyFrame keyFrame in m_keyFrames) {
            keyFrame.SetOwner(this);
            keyFrame.RefreshMarkerOwner();
        }
    }    
    #endregion
//----------------------------------------------------------------------------------------------------------------------
    internal override void DestroyV() {

        foreach (PlayableKeyFrame keyFrame in m_keyFrames) {
            keyFrame.Destroy();
        }

    }
    

//----------------------------------------------------------------------------------------------------------------------

    internal bool AreKeyFrameMarkersRequested() {
        return m_keyFrameMarkersRequested;
    }

    internal void RequestKeyFrameMarkers(bool req, bool forceShow = false) {

        if (req == m_keyFrameMarkersRequested)
            return;
        
#if UNITY_EDITOR
        Undo.RegisterCompleteObjectUndo(GetOwner().GetParentTrack(),"MeshSync: Show/Hide Key Frame Markers");
        m_forceShowFrameMarkers = forceShow && req;
#endif        
        m_keyFrameMarkersRequested = req;
        if (UpdateFrameMarkersVisibility()) {
            RefreshKeyFrames();
        }
    }

    internal int GetNumKeyFrames() { return m_keyFrames.Count;}


#if UNITY_EDITOR
    internal void UpdateTimelineWidthPerFrame(float visibleRectWidth, double visibleTime, double fps, double timeScale) {
        int numFrames = Mathf.RoundToInt((float)
            ((visibleTime) * fps / timeScale) 
        );
                
        double widthPerFrame = visibleRectWidth / numFrames;
        m_timelineWidthPerFrame = widthPerFrame;
        if (UpdateFrameMarkersVisibility()) {
            RefreshKeyFrames();
            
        }                
    }
    
#endif    
    
//----------------------------------------------------------------------------------------------------------------------    
    private static PlayableKeyFrame CreateKeyFrame(KeyFrameControllerClipData owner, int index, double timePerFrame) 
    {
        PlayableKeyFrame playableKeyFrame = new PlayableKeyFrame(owner);
        playableKeyFrame.SetIndexAndLocalTime(index, timePerFrame * index);
        playableKeyFrame.SetPlayFrame(index);
        return playableKeyFrame;
    }

//----------------------------------------------------------------------------------------------------------------------
    private void DestroyPlayableFrames() {
        if (null == m_keyFrames)
            return;
        
        foreach (PlayableKeyFrame frame in m_keyFrames) {
            frame?.Destroy();
        }        
    } 
    
    internal void RegenerateKeyFrames(int span, KeyFrameMode mode) {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);

        int numIdealNumPlayableFrames = CalculateNumIdealKeyFrames(clip);
        UpdateKeyFramesSize(numIdealNumPlayableFrames);
        InitKeyFrames(0, m_keyFrames.Count, span, mode);
    }
    
    internal void RegenerateKeyFrames(int startIndex, int endIndex, int span, KeyFrameMode mode) {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);

        int numIdealNumPlayableFrames = CalculateNumIdealKeyFrames(clip);
        UpdateKeyFramesSize(numIdealNumPlayableFrames);
        
        InitKeyFrames(startIndex, endIndex, span, mode);
    }

    internal void AddKeyFrame(double globalTime) {
        TimelineClip clip = GetOwnerIfReady();
        if (null == clip)
            return;

        int              frameIndex = LocalTimeToFrameIndex(clip.ToLocalTime(globalTime), clip.duration, clip.clipIn);
        PlayableKeyFrame keyFrame   = m_keyFrames[frameIndex];
        if (keyFrame.IsEnabled())
            return;
        
        keyFrame.SetEnabled(true);
        keyFrame.SetUserNote("");
        
        keyFrame.SetKeyFrameMode(KeyFrameMode.Continuous);
        
        PlayableKeyFrame prevEnabledKeyFrame = FindEnabledKeyFrame(m_keyFrames, frameIndex - 1, 0);
        if (null == prevEnabledKeyFrame)
            prevEnabledKeyFrame = m_keyFrames[0];

        if (KeyFrameMode.Hold == prevEnabledKeyFrame.GetKeyFrameMode()) {
            keyFrame.SetPlayFrame(prevEnabledKeyFrame.GetPlayFrame());
        } else {
            
            PlayableKeyFrame nextEnabledKeyFrame = FindEnabledKeyFrame(m_keyFrames, frameIndex + 1, m_keyFrames.Count);
            if (null == nextEnabledKeyFrame)
                nextEnabledKeyFrame = m_keyFrames[frameIndex];
        
            AnimationCurve linearCurve = AnimationCurve.Linear((float)prevEnabledKeyFrame.GetLocalTime(), prevEnabledKeyFrame.GetPlayFrame(), 
                (float)nextEnabledKeyFrame.GetLocalTime(), nextEnabledKeyFrame.GetPlayFrame());
            int frameNo = (int )linearCurve.Evaluate((float)keyFrame.GetLocalTime());
            keyFrame.SetPlayFrame(frameNo);
        }
        
        if (m_keyFrameMarkersVisibility)
            keyFrame.RefreshMarker(m_keyFrameMarkersVisibility);
        
        
    }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal void OnGraphStart() {
#if UNITY_EDITOR
        if (m_needToRefreshTimelineEditor) {
            foreach (PlayableKeyFrame playableKeyFrame in m_keyFrames) {
                playableKeyFrame.RefreshMarker(m_keyFrameMarkersVisibility);
            }
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved);
        }
#endif        
    }

    internal void OnClipChanged() {
        
        TimelineClip clipOwner = GetOwnerIfReady();
        if (null == clipOwner)
            return;
        
        //Need to decide if we want apply the KeyFrames data to Marker, or the other way around        
        if (NeedsRefreshKeyFrames() || UpdateFrameMarkersVisibility()) {
            RefreshKeyFrames();
            return;
        }

        if (!m_keyFrameMarkersVisibility) 
            return;
        
        //Find KeyFrames that need to be moved
        int                           numKeyFrames       = m_keyFrames.Count;
        Dictionary<int, KeyFrameInfo> movedKeyFrames     = new Dictionary<int, KeyFrameInfo>();
        HashSet<int>                  keyFramesToDisable = new HashSet<int>();
        
        for (int i = 0; i < numKeyFrames; ++i) {
            PlayableKeyFrame keyFrame = m_keyFrames[i];
            keyFrame.SaveStateFromMarker();
            int moveDestIndex = LocalTimeToFrameIndex(keyFrame.GetLocalTime(), clipOwner.duration, clipOwner.clipIn);            
            if (moveDestIndex == i)
                continue;

            movedKeyFrames[moveDestIndex] = (new KeyFrameInfo() {
                enabled = true,
                //localTime = keyFrame.GetLocalTime(),
                mode      = keyFrame.GetKeyFrameMode(),
                playFrame = keyFrame.GetPlayFrame(),
            });
            

            keyFramesToDisable.Add(i);

            //KeyFrameInfo info = keyFrameInfoSet[applicableIndex];            
            //Debug.Log($"{applicableIndex}, {i}, {numPlayableFrames}, {info.enabled} {info.frameNo} {info.mode} {keyFrame.GetLocalTime()} {keyFrame.GetLocalTime() / clipOwner.duration}");
        }

        foreach (int moveDestIndex in movedKeyFrames.Keys) {
            keyFramesToDisable.Remove(moveDestIndex);
        }

        //Update PlayableFrames structure back
        double timePerFrame = FilmInternalUtilities.TimelineUtility.CalculateTimePerFrame(clipOwner);
        for (int i = 0; i < numKeyFrames; ++i) {
            m_keyFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            if (movedKeyFrames.TryGetValue(i, out KeyFrameInfo keyFrameInfo)) {
                
                //Debug.Log($"Setting {i} {keyFrameInfo.enabled} {keyFrameInfo.frameNo} {keyFrameInfo.mode} ");
                m_keyFrames[i].SetPlayFrame(keyFrameInfo.playFrame);
                m_keyFrames[i].SetEnabled(keyFrameInfo.enabled);
                m_keyFrames[i].SetKeyFrameMode(keyFrameInfo.mode);
            }
        }

        foreach (int i in keyFramesToDisable) {
            m_keyFrames[i].SetEnabled(false);
        }
        
        if (movedKeyFrames.Count > 0) {
            m_needToRefreshTimelineEditor = true;
        }
    }

    internal void ResetKeyFrames() {
        TimelineClip clipOwner = GetOwnerIfReady();
        if (null == clipOwner)
            return;

        int numIdealNumPlayableFrames = CalculateNumIdealKeyFrames(clipOwner);
        UpdateKeyFramesSize(numIdealNumPlayableFrames);
        
        InitKeyFrames(0, numIdealNumPlayableFrames-1, span: 1, KeyFrameMode.Continuous);
    }
    
    private void InitKeyFrames(int startIndex, int endIndex, int span, KeyFrameMode mode) {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
        
        double timePerFrame = FilmInternalUtilities.TimelineUtility.CalculateTimePerFrame(clip);
        endIndex = Mathf.Min(endIndex, m_keyFrames.Count);
        for (int i = startIndex; i < endIndex; ++i) {
            m_keyFrames[i].SetOwner(this);
            m_keyFrames[i].SetEnabled( i % span == 0);
            m_keyFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_keyFrames[i].SetPlayFrame(i);
            m_keyFrames[i].SetKeyFrameMode(mode);
            m_keyFrames[i].RefreshMarker(m_keyFrameMarkersVisibility);
        }
        
    }


    private bool NeedsRefreshKeyFrames() {
        TimelineClip clip = GetOwnerIfReady();
        if (null == clip) {
            return false;
        }
        
        if (!Mathf.Approximately(m_lastClipStartTimeOnRefresh, (float) clip.start)) {
            return true;
        }

        if (!Mathf.Approximately(m_lastClipDurationOnRefresh, (float) clip.duration)) {
            return true;
        }

        if (!Mathf.Approximately(m_lastClipTimeScaleOnRefresh, (float) clip.timeScale)) {
            return true;
        }

        if (!Mathf.Approximately(m_lastClipInOnRefresh, (float) clip.clipIn)) {
            return true;
        }
        
        int numIdealNumPlayableFrames = CalculateNumIdealKeyFrames(clip);
        if (numIdealNumPlayableFrames != m_keyFrames.Count)
            return true;
        
        return false;

    }
    
    //Resize PlayableFrames and used the previous values
    private void RefreshKeyFrames() {

        TimelineClip clipOwner = GetOwnerIfReady();
        if (null == clipOwner) {
            return;
        }
        
        m_lastClipStartTimeOnRefresh = (float) clipOwner.start;
        m_lastClipDurationOnRefresh  = (float) clipOwner.duration;
        m_lastClipTimeScaleOnRefresh = (float) clipOwner.timeScale;
        m_lastClipInOnRefresh        = (float) clipOwner.clipIn;
        
        int numIdealNumPlayableFrames = CalculateNumIdealKeyFrames(clipOwner);
        
      
        //Change the size of m_playableFrames and reinitialize if necessary
        int prevNumPlayableFrames = m_keyFrames.Count;
        
        if (numIdealNumPlayableFrames != prevNumPlayableFrames) {
            
            //Change the size of m_playableFrames and reinitialize if necessary
            List<KeyFrameMode> prevKeyFrameModes = new List<KeyFrameMode>(prevNumPlayableFrames);
            foreach (PlayableKeyFrame frame in m_keyFrames) {
                //if frame ==null, just use the default
                prevKeyFrameModes.Add(null == frame ? KeyFrameMode.Continuous : frame.GetKeyFrameMode()); 
            }

            UpdateKeyFramesSize(numIdealNumPlayableFrames);
            
            //Reinitialize 
            if (prevNumPlayableFrames > 0) {
                int minNumPlayableFrames = Math.Min(prevNumPlayableFrames, m_keyFrames.Count);
                for (int i = 0; i < minNumPlayableFrames; ++i) {
                    m_keyFrames[i].SetKeyFrameMode(prevKeyFrameModes[i]);
                }
            }
        }
        
        //Refresh all markers
        double timePerFrame      = FilmInternalUtilities.TimelineUtility.CalculateTimePerFrame(clipOwner);
        int    numPlayableFrames = m_keyFrames.Count;
        for (int i = 0; i < numPlayableFrames; ++i) {
            m_keyFrames[i].SetOwner(this);
            m_keyFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_keyFrames[i].RefreshMarker(m_keyFrameMarkersVisibility);
        }
        
    }        
    
    
//----------------------------------------------------------------------------------------------------------------------
    [CanBeNull]
    internal PlayableKeyFrame GetKeyFrame(int index) {
        if (null == m_keyFrames || index >= m_keyFrames.Count)
            return null;
        
        Assert.IsTrue(null!=m_keyFrames && index < m_keyFrames.Count);
        return m_keyFrames[index];
    }


//----------------------------------------------------------------------------------------------------------------------
    
    private void UpdateKeyFramesSize(int reqPlayableFramesSize) {
        TimelineClip clipOwner = GetOwner();
        Assert.IsNotNull(clipOwner);

        double timePerFrame = FilmInternalUtilities.TimelineUtility.CalculateTimePerFrame(clipOwner);
        //Resize m_playableFrames
        if (m_keyFrames.Count < reqPlayableFramesSize) {
            int             numNewPlayableFrames = (reqPlayableFramesSize - m_keyFrames.Count);
            List<PlayableKeyFrame> newPlayableFrames = new List<PlayableKeyFrame>(numNewPlayableFrames);           
            for (int i = m_keyFrames.Count; i < reqPlayableFramesSize; ++i) {
                newPlayableFrames.Add(CreateKeyFrame(this,i, timePerFrame));
            }            
            m_keyFrames.AddRange(newPlayableFrames);                
        }

        if (m_keyFrames.Count > reqPlayableFramesSize) {
            int numLastPlayableFrames = m_keyFrames.Count;
            for (int i = reqPlayableFramesSize; i < numLastPlayableFrames; ++i) {
                PlayableKeyFrame curKeyFrame = m_keyFrames[i];
                curKeyFrame?.Destroy();
            }            
            m_keyFrames.RemoveRange(reqPlayableFramesSize, numLastPlayableFrames - reqPlayableFramesSize);
        }
            
        Assert.IsTrue(m_keyFrames.Count == reqPlayableFramesSize);
           
        for (int i = 0; i < reqPlayableFramesSize; ++i) {
            PlayableKeyFrame curPlayableKeyFrame = m_keyFrames[i];
            Assert.IsNotNull(curPlayableKeyFrame);
            m_keyFrames[i].SetIndexAndLocalTime(i, timePerFrame * i);
            
        }
    }

//----------------------------------------------------------------------------------------------------------------------

    //return true if the visibility has changed
    private bool UpdateFrameMarkersVisibility() {
        
        bool prevVisibility = m_keyFrameMarkersVisibility;
#if UNITY_EDITOR        
        const int FRAME_MARKER_WIDTH_THRESHOLD = 10; //less: tend to keep drawing markers
        m_keyFrameMarkersVisibility = m_keyFrameMarkersRequested && 
            (m_forceShowFrameMarkers || m_timelineWidthPerFrame > FRAME_MARKER_WIDTH_THRESHOLD);
#else
        m_keyFrameMarkersVisibility = m_keyFrameMarkersRequested;
#endif
        return prevVisibility != m_keyFrameMarkersVisibility;
    }

    [CanBeNull]
    private TimelineClip GetOwnerIfReady() {
        TimelineClip clipOwner = GetOwner();

        //Clip may not have a parent because the clip is being moved 
        return null == clipOwner.GetParentTrack() ? null : clipOwner;
    }

    private int LocalTimeToFrameIndex(double localTime, double clipDuration, double clipIn) {
        int numKeyFrames = m_keyFrames.Count;
        int index = Mathf.RoundToInt((float)(localTime * numKeyFrames / (clipDuration + clipIn)));
        index = Mathf.Clamp(index,0,numKeyFrames - 1);
        return index;
    }


    [CanBeNull]
    static PlayableKeyFrame FindEnabledKeyFrame(IList<PlayableKeyFrame> keyFrames, int startIndex, int endIndex) {

        int counter = (int)Mathf.Sign(endIndex - startIndex);
        for (int i = startIndex; i != endIndex; i += counter) {
            if (keyFrames[i].IsEnabled()) {
                return keyFrames[i];
            }
        }
        return null;
        
    }

    private int CalculateNumIdealKeyFrames(TimelineClip clip) {
        double fps               = clip.GetParentTrack().timelineAsset.editorSettings.GetFPS();
        int    numIdealKeyFrames = Mathf.RoundToInt((float)((clip.duration + clip.clipIn) * fps));
        return numIdealKeyFrames; 
        
    }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //The ground truth for using/dropping an image in a particular frame. See the notes below
    [SerializeField] private List<PlayableKeyFrame> m_keyFrames;
    [SerializeField] [HideInInspector] private bool m_keyFrameMarkersRequested = false;

    private float m_lastClipStartTimeOnRefresh = 0;
    private float m_lastClipDurationOnRefresh  = 0;
    private float m_lastClipTimeScaleOnRefresh = 0;
    private float m_lastClipInOnRefresh        = 0;

    
#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_keyframeControllerClipDataVersion = CUR_KEYFRAME_CONTROLLER_CLIP_DATA_VERSION;        
#pragma warning restore 414    
    
#if UNITY_EDITOR    
    private double m_timelineWidthPerFrame = Int16.MaxValue;
    private bool   m_forceShowFrameMarkers = false;
#endif

    private bool m_keyFrameMarkersVisibility   = false;
    private bool m_needToRefreshTimelineEditor = false;
    
    private const int    CUR_KEYFRAME_CONTROLLER_CLIP_DATA_VERSION = 1;
    
}


} //end namespace

