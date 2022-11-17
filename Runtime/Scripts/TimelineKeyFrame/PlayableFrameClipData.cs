using System;
using System.Collections.Generic;
using JetBrains.Annotations;
using Unity.FilmInternalUtilities;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync {

[Serializable]
internal abstract class PlayableFrameClipData : BaseClipData {

    protected PlayableFrameClipData() {
        m_playableFrames = new List<PlayableKeyFrame>();
    }

    protected PlayableFrameClipData(TimelineClip clipOwner) {
        SetOwner(clipOwner);
        m_playableFrames = new List<PlayableKeyFrame>();
    }

    protected PlayableFrameClipData(TimelineClip owner, PlayableFrameClipData other) : this(owner){
        Assert.IsNotNull(m_playableFrames);
        
        foreach (PlayableKeyFrame otherFrame in other.m_playableFrames) {
            PlayableKeyFrame newKeyFrame = new PlayableKeyFrame(this, otherFrame);
            m_playableFrames.Add(newKeyFrame);
        }
        
        m_frameMarkersRequested = other.m_frameMarkersRequested;
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    protected override void OnBeforeSerializeInternalV() {
    }

    protected override void OnAfterDeserializeInternalV() {
        foreach (PlayableKeyFrame playableFrame in m_playableFrames) {
            playableFrame.SetOwner(this);
        }
    }    
    #endregion
//----------------------------------------------------------------------------------------------------------------------
    internal override void DestroyV() {

        foreach (PlayableKeyFrame playableFrame in m_playableFrames) {
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
    private static PlayableKeyFrame CreatePlayableFrame(PlayableFrameClipData owner, int index, double timePerFrame) 
    {
        PlayableKeyFrame playableKeyFrame = new PlayableKeyFrame(owner);
        playableKeyFrame.SetIndexAndLocalTime(index, timePerFrame * index);
        playableKeyFrame.SetPlayFrame(index);
        return playableKeyFrame;
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    internal void ResetPlayableFrames() {
        DestroyPlayableFrames();

        //Recalculate the number of frames and create the marker's ground truth data
        int numFrames = TimelineUtility.CalculateNumFrames(GetOwner());
        m_playableFrames = new List<PlayableKeyFrame>(numFrames);
        UpdatePlayableFramesSize(numFrames);                
    }


//----------------------------------------------------------------------------------------------------------------------
    private void DestroyPlayableFrames() {
        if (null == m_playableFrames)
            return;
        
        foreach (PlayableKeyFrame frame in m_playableFrames) {
            frame?.Destroy();
        }        
    } 
    
    internal void RegenerateKeyFrames(int span, KeyFrameMode mode) {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);

        int numIdealNumPlayableFrames = TimelineUtility.CalculateNumFrames(clip);
        UpdatePlayableFramesSize(numIdealNumPlayableFrames);
        InitKeyFrames(0, m_playableFrames.Count, span, mode);
    }
    
    internal void RegenerateKeyFrames(int startIndex, int endIndex, int span, KeyFrameMode mode) {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);

        int numIdealNumPlayableFrames = TimelineUtility.CalculateNumFrames(clip);
        UpdatePlayableFramesSize(numIdealNumPlayableFrames);
        
        InitKeyFrames(startIndex, endIndex, span, mode);
    }

    private void InitKeyFrames(int startIndex, int endIndex, int span, KeyFrameMode mode) {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
        
        double timePerFrame = TimelineUtility.CalculateTimePerFrame(clip);
        endIndex = Mathf.Min(endIndex, m_playableFrames.Count);
        for (int i = startIndex; i < endIndex; ++i) {
            m_playableFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_playableFrames[i].SetPlayFrame(i);
            m_playableFrames[i].SetEnabled( i % span == 0);
            m_playableFrames[i].SetKeyFrameMode(mode);
            m_playableFrames[i].RefreshMarker(m_frameMarkersVisibility);
        }
        
    }
    
    

    internal void AddKeyFrame(double globalTime) {
        TimelineClip clip = GetOwnerIfReady();
        if (null == clip)
            return;
            
        //already enabled
        int              frameIndex = LocalTimeToFrameIndex(globalTime - clip.start, clip.duration);
        PlayableKeyFrame keyFrame   = m_playableFrames[frameIndex];
        if (keyFrame.IsEnabled())
            return;
        
        keyFrame.SetEnabled(true);
        keyFrame.SetUserNote("");
        
        keyFrame.SetKeyFrameMode(KeyFrameMode.Continuous);
        
        PlayableKeyFrame prevEnabledKeyFrame = FindEnabledKeyFrame(m_playableFrames, frameIndex - 1, 0);
        if (null == prevEnabledKeyFrame)
            prevEnabledKeyFrame = m_playableFrames[0];

        if (KeyFrameMode.Hold == prevEnabledKeyFrame.GetKeyFrameMode()) {
            keyFrame.SetPlayFrame(prevEnabledKeyFrame.GetPlayFrame());
        } else {
            
            PlayableKeyFrame nextEnabledKeyFrame = FindEnabledKeyFrame(m_playableFrames, frameIndex + 1, m_playableFrames.Count);
            if (null == nextEnabledKeyFrame)
                nextEnabledKeyFrame = m_playableFrames[frameIndex];
        
            AnimationCurve linearCurve = AnimationCurve.Linear((float)prevEnabledKeyFrame.GetLocalTime(), prevEnabledKeyFrame.GetPlayFrame(), 
                (float)nextEnabledKeyFrame.GetLocalTime(), nextEnabledKeyFrame.GetPlayFrame());
            int frameNo = (int )linearCurve.Evaluate((float)keyFrame.GetLocalTime());
            keyFrame.SetPlayFrame(frameNo);
        }
        
        if (m_frameMarkersVisibility)
            keyFrame.RefreshMarker(m_frameMarkersVisibility);
        
        
    }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal void OnGraphStart() {
        if (m_needToRefreshTimelineEditor) {
            foreach (PlayableKeyFrame playableKeyFrame in m_playableFrames) {
                playableKeyFrame.RefreshMarker(m_frameMarkersVisibility);
            }
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved);
        }
    }

    internal void OnClipChanged() {
        
        TimelineClip clipOwner = GetOwnerIfReady();
        if (null == clipOwner)
            return;
        
        if (NeedsRefreshPlayableFrames()) {
            RefreshPlayableFrames();
            return;
        }
        
        if (!m_frameMarkersVisibility) 
            return;
        
        //Find KeyFrames that need to be moved
        int                           numPlayableFrames     = m_playableFrames.Count;
        Dictionary<int, KeyFrameInfo> movedKeyFrames        = new Dictionary<int, KeyFrameInfo>();
        HashSet<int>                  keyFramesToDisable    = new HashSet<int>();
        
        
        for (int i = 0; i < numPlayableFrames; ++i) {
            PlayableKeyFrame keyKeyFrame = m_playableFrames[i];
            keyKeyFrame.SaveStateFromMarker();
            int moveDestIndex = Mathf.RoundToInt((float)(keyKeyFrame.GetLocalTime() * numPlayableFrames / clipOwner.duration));
            moveDestIndex = Mathf.Clamp(moveDestIndex,0,numPlayableFrames - 1);
            
            if (moveDestIndex == i)
                continue;
            

            movedKeyFrames[moveDestIndex] = (new KeyFrameInfo() {
                enabled = true,
                //localTime = keyFrame.GetLocalTime(),
                mode    = keyKeyFrame.GetKeyFrameMode(),
                playFrame = keyKeyFrame.GetPlayFrame(),
            });
            

            keyFramesToDisable.Add(i);

            //KeyFrameInfo info = keyFrameInfoSet[applicableIndex];            
            //Debug.Log($"{applicableIndex}, {i}, {numPlayableFrames}, {info.enabled} {info.frameNo} {info.mode} {keyFrame.GetLocalTime()} {keyFrame.GetLocalTime() / clipOwner.duration}");
        }

        foreach (int moveDestIndex in movedKeyFrames.Keys) {
            keyFramesToDisable.Remove(moveDestIndex);
        }

        //Update PlayableFrames structure back
        double timePerFrame = TimelineUtility.CalculateTimePerFrame(clipOwner);
        for (int i = 0; i < numPlayableFrames; ++i) {
            m_playableFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            if (movedKeyFrames.TryGetValue(i, out KeyFrameInfo keyFrameInfo)) {
                
                //Debug.Log($"Setting {i} {keyFrameInfo.enabled} {keyFrameInfo.frameNo} {keyFrameInfo.mode} ");
                m_playableFrames[i].SetPlayFrame(keyFrameInfo.playFrame);
                m_playableFrames[i].SetEnabled(keyFrameInfo.enabled);
                m_playableFrames[i].SetKeyFrameMode(keyFrameInfo.mode);
            }
        }

        foreach (int i in keyFramesToDisable) {
            m_playableFrames[i].SetEnabled(false);
        }
        
        if (movedKeyFrames.Count > 0) {
            m_needToRefreshTimelineEditor = true;
        }
    }

    internal void InitPlayableFrames() {
        TimelineClip clipOwner = GetOwnerIfReady();
        if (null == clipOwner)
            return;

        int numIdealNumPlayableFrames = TimelineUtility.CalculateNumFrames(clipOwner);
        
        UpdatePlayableFramesSize(numIdealNumPlayableFrames);
        
        //Refresh all markers
        double timePerFrame      = TimelineUtility.CalculateTimePerFrame(clipOwner);
        int    numPlayableFrames = m_playableFrames.Count;
        for (int i = 0; i < numPlayableFrames; ++i) {
            m_playableFrames[i].SetEnabled(true);
            m_playableFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_playableFrames[i].SetPlayFrame(i);
            m_playableFrames[i].SetKeyFrameMode(KeyFrameMode.Continuous);
            m_playableFrames[i].RefreshMarker(m_frameMarkersVisibility);
        }
    }

    private bool NeedsRefreshPlayableFrames() {
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
        
        int numIdealNumPlayableFrames = TimelineUtility.CalculateNumFrames(clip);
        if (numIdealNumPlayableFrames != m_playableFrames.Count)
            return true;
        
        return false;

    }
    
    //Resize PlayableFrames and used the previous values
    private void RefreshPlayableFrames() {

        TimelineClip clipOwner = GetOwnerIfReady();
        if (null == clipOwner) {
            return;
        }
        
        m_lastClipStartTimeOnRefresh = (float) clipOwner.start;
        m_lastClipDurationOnRefresh  = (float) clipOwner.duration;
        
        int numIdealNumPlayableFrames = TimelineUtility.CalculateNumFrames(clipOwner);
      
        //Change the size of m_playableFrames and reinitialize if necessary
        int prevNumPlayableFrames = m_playableFrames.Count;
        if (numIdealNumPlayableFrames != prevNumPlayableFrames) {
            
            //Change the size of m_playableFrames and reinitialize if necessary
            List<KeyFrameMode> prevKeyFrameModes = new List<KeyFrameMode>(prevNumPlayableFrames);
            foreach (PlayableKeyFrame frame in m_playableFrames) {
                //if frame ==null, just use the default
                prevKeyFrameModes.Add(null == frame ? KeyFrameMode.Continuous : frame.GetKeyFrameMode()); 
            }

            UpdatePlayableFramesSize(numIdealNumPlayableFrames);
            
            //Reinitialize 
            if (prevNumPlayableFrames > 0) {
                int minNumPlayableFrames = Math.Min(prevNumPlayableFrames, m_playableFrames.Count);
                for (int i = 0; i < minNumPlayableFrames; ++i) {
                    m_playableFrames[i].SetKeyFrameMode(prevKeyFrameModes[i]);
                }
            }
        }
        
        //Refresh all markers
        double timePerFrame           = TimelineUtility.CalculateTimePerFrame(clipOwner);                
        int    numPlayableFrames      = m_playableFrames.Count;
        for (int i = 0; i < numPlayableFrames; ++i) {
            m_playableFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_playableFrames[i].RefreshMarker(m_frameMarkersVisibility);
        }
        
    }        
    
    
//----------------------------------------------------------------------------------------------------------------------
    //may return null
    internal PlayableKeyFrame GetPlayableFrame(int index) {
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
            List<PlayableKeyFrame> newPlayableFrames = new List<PlayableKeyFrame>(numNewPlayableFrames);           
            for (int i = m_playableFrames.Count; i < reqPlayableFramesSize; ++i) {
                newPlayableFrames.Add(CreatePlayableFrame(this,i, timePerFrame));
            }            
            m_playableFrames.AddRange(newPlayableFrames);                
        }

        if (m_playableFrames.Count > reqPlayableFramesSize) {
            int numLastPlayableFrames = m_playableFrames.Count;
            for (int i = reqPlayableFramesSize; i < numLastPlayableFrames; ++i) {
                PlayableKeyFrame curKeyFrame = m_playableFrames[i];
                curKeyFrame?.Destroy();
            }            
            m_playableFrames.RemoveRange(reqPlayableFramesSize, numLastPlayableFrames - reqPlayableFramesSize);
        }
            
        Assert.IsTrue(m_playableFrames.Count == reqPlayableFramesSize);
           
        for (int i = 0; i < reqPlayableFramesSize; ++i) {
            PlayableKeyFrame curPlayableKeyFrame = m_playableFrames[i];
            Assert.IsNotNull(curPlayableKeyFrame);                
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

    [CanBeNull]
    private TimelineClip GetOwnerIfReady() {
        TimelineClip clipOwner = GetOwner();

        //Clip may not have a parent because the clip is being moved 
        return null == clipOwner.GetParentTrack() ? null : clipOwner;
    }

    private int LocalTimeToFrameIndex(double localTime, double clipDuration) {
        int numPlayableFrames = m_playableFrames.Count;
        int index   = Mathf.RoundToInt((float)(localTime * numPlayableFrames / clipDuration));
        index = Mathf.Clamp(index,0,numPlayableFrames - 1);
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
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //The ground truth for using/dropping an image in a particular frame. See the notes below
    [SerializeField] private List<PlayableKeyFrame> m_playableFrames;
    [SerializeField] [HideInInspector] private bool m_frameMarkersRequested = true;

    [SerializeField] [HideInInspector] private float m_lastClipStartTimeOnRefresh = 0;
    [SerializeField] [HideInInspector] private float m_lastClipDurationOnRefresh = 0;

    
#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_playableFrameClipDataVersion = CUR_PLAYABLE_FRAME_CLIP_DATA_VERSION;        
#pragma warning restore 414    
    
#if UNITY_EDITOR    
    private double m_timelineWidthPerFrame = Int16.MaxValue;
    private bool   m_forceShowFrameMarkers = false;
#endif

    private bool m_frameMarkersVisibility      = true;
    private bool m_needToRefreshTimelineEditor = false;
    
    private const int    CUR_PLAYABLE_FRAME_CLIP_DATA_VERSION = 1;
    
}


} //end namespace

