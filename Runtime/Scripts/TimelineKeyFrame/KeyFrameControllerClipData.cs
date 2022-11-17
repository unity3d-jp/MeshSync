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
internal abstract class KeyFrameControllerClipData : BaseClipData {

    protected KeyFrameControllerClipData() {
        m_playableKeyFrames = new List<PlayableKeyFrame>();
    }

    protected KeyFrameControllerClipData(TimelineClip clipOwner) {
        SetOwner(clipOwner);
        m_playableKeyFrames = new List<PlayableKeyFrame>();
    }

    protected KeyFrameControllerClipData(TimelineClip owner, KeyFrameControllerClipData other) : this(owner){
        Assert.IsNotNull(m_playableKeyFrames);
        
        foreach (PlayableKeyFrame otherFrame in other.m_playableKeyFrames) {
            PlayableKeyFrame newKeyFrame = new PlayableKeyFrame(this, otherFrame);
            m_playableKeyFrames.Add(newKeyFrame);
        }
        
        m_keyFrameMarkersRequested = other.m_keyFrameMarkersRequested;
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    #region ISerializationCallbackReceiver
    protected override void OnBeforeSerializeInternalV() {
    }

    protected override void OnAfterDeserializeInternalV() {
        foreach (PlayableKeyFrame playableFrame in m_playableKeyFrames) {
            playableFrame.SetOwner(this);
        }
    }    
    #endregion
//----------------------------------------------------------------------------------------------------------------------
    internal override void DestroyV() {

        foreach (PlayableKeyFrame playableFrame in m_playableKeyFrames) {
            playableFrame.Destroy();
        }

    }
    

//----------------------------------------------------------------------------------------------------------------------

    internal bool AreFrameMarkersRequested() {
        return m_keyFrameMarkersRequested;
    }

    internal void RequestFrameMarkers(bool req, bool forceShow = false) {

        if (req == m_keyFrameMarkersRequested)
            return;
        
#if UNITY_EDITOR
        Undo.RegisterCompleteObjectUndo(GetOwner().GetParentTrack(),"StreamingImageSequence Show/Hide FrameMarker");
        m_forceShowFrameMarkers = forceShow && req;
#endif        
        m_keyFrameMarkersRequested = req;
        if (UpdateFrameMarkersVisibility()) {
            RefreshPlayableFrames();                    
        }
    }

    internal int GetNumPlayableFrames() { return m_playableKeyFrames.Count;}


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
    private static PlayableKeyFrame CreatePlayableFrame(KeyFrameControllerClipData owner, int index, double timePerFrame) 
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
        m_playableKeyFrames = new List<PlayableKeyFrame>(numFrames);
        UpdatePlayableFramesSize(numFrames);                
    }


//----------------------------------------------------------------------------------------------------------------------
    private void DestroyPlayableFrames() {
        if (null == m_playableKeyFrames)
            return;
        
        foreach (PlayableKeyFrame frame in m_playableKeyFrames) {
            frame?.Destroy();
        }        
    } 
    
    internal void RegenerateKeyFrames(int span, KeyFrameMode mode) {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);

        int numIdealNumPlayableFrames = TimelineUtility.CalculateNumFrames(clip);
        UpdatePlayableFramesSize(numIdealNumPlayableFrames);
        InitKeyFrames(0, m_playableKeyFrames.Count, span, mode);
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
        endIndex = Mathf.Min(endIndex, m_playableKeyFrames.Count);
        for (int i = startIndex; i < endIndex; ++i) {
            m_playableKeyFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_playableKeyFrames[i].SetPlayFrame(i);
            m_playableKeyFrames[i].SetEnabled( i % span == 0);
            m_playableKeyFrames[i].SetKeyFrameMode(mode);
            m_playableKeyFrames[i].RefreshMarker(m_keyFrameMarkersVisibility);
        }
        
    }
    
    

    internal void AddKeyFrame(double globalTime) {
        TimelineClip clip = GetOwnerIfReady();
        if (null == clip)
            return;
            
        //already enabled
        int              frameIndex = LocalTimeToFrameIndex(globalTime - clip.start, clip.duration);
        PlayableKeyFrame keyFrame   = m_playableKeyFrames[frameIndex];
        if (keyFrame.IsEnabled())
            return;
        
        keyFrame.SetEnabled(true);
        keyFrame.SetUserNote("");
        
        keyFrame.SetKeyFrameMode(KeyFrameMode.Continuous);
        
        PlayableKeyFrame prevEnabledKeyFrame = FindEnabledKeyFrame(m_playableKeyFrames, frameIndex - 1, 0);
        if (null == prevEnabledKeyFrame)
            prevEnabledKeyFrame = m_playableKeyFrames[0];

        if (KeyFrameMode.Hold == prevEnabledKeyFrame.GetKeyFrameMode()) {
            keyFrame.SetPlayFrame(prevEnabledKeyFrame.GetPlayFrame());
        } else {
            
            PlayableKeyFrame nextEnabledKeyFrame = FindEnabledKeyFrame(m_playableKeyFrames, frameIndex + 1, m_playableKeyFrames.Count);
            if (null == nextEnabledKeyFrame)
                nextEnabledKeyFrame = m_playableKeyFrames[frameIndex];
        
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
        if (m_needToRefreshTimelineEditor) {
            foreach (PlayableKeyFrame playableKeyFrame in m_playableKeyFrames) {
                playableKeyFrame.RefreshMarker(m_keyFrameMarkersVisibility);
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
        
        if (!m_keyFrameMarkersVisibility) 
            return;
        
        //Find KeyFrames that need to be moved
        int                           numPlayableFrames     = m_playableKeyFrames.Count;
        Dictionary<int, KeyFrameInfo> movedKeyFrames        = new Dictionary<int, KeyFrameInfo>();
        HashSet<int>                  keyFramesToDisable    = new HashSet<int>();
        
        
        for (int i = 0; i < numPlayableFrames; ++i) {
            PlayableKeyFrame keyKeyFrame = m_playableKeyFrames[i];
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
            m_playableKeyFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            if (movedKeyFrames.TryGetValue(i, out KeyFrameInfo keyFrameInfo)) {
                
                //Debug.Log($"Setting {i} {keyFrameInfo.enabled} {keyFrameInfo.frameNo} {keyFrameInfo.mode} ");
                m_playableKeyFrames[i].SetPlayFrame(keyFrameInfo.playFrame);
                m_playableKeyFrames[i].SetEnabled(keyFrameInfo.enabled);
                m_playableKeyFrames[i].SetKeyFrameMode(keyFrameInfo.mode);
            }
        }

        foreach (int i in keyFramesToDisable) {
            m_playableKeyFrames[i].SetEnabled(false);
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
        int    numPlayableFrames = m_playableKeyFrames.Count;
        for (int i = 0; i < numPlayableFrames; ++i) {
            m_playableKeyFrames[i].SetEnabled(true);
            m_playableKeyFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_playableKeyFrames[i].SetPlayFrame(i);
            m_playableKeyFrames[i].SetKeyFrameMode(KeyFrameMode.Continuous);
            m_playableKeyFrames[i].RefreshMarker(m_keyFrameMarkersVisibility);
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
        if (numIdealNumPlayableFrames != m_playableKeyFrames.Count)
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
        int prevNumPlayableFrames = m_playableKeyFrames.Count;
        if (numIdealNumPlayableFrames != prevNumPlayableFrames) {
            
            //Change the size of m_playableFrames and reinitialize if necessary
            List<KeyFrameMode> prevKeyFrameModes = new List<KeyFrameMode>(prevNumPlayableFrames);
            foreach (PlayableKeyFrame frame in m_playableKeyFrames) {
                //if frame ==null, just use the default
                prevKeyFrameModes.Add(null == frame ? KeyFrameMode.Continuous : frame.GetKeyFrameMode()); 
            }

            UpdatePlayableFramesSize(numIdealNumPlayableFrames);
            
            //Reinitialize 
            if (prevNumPlayableFrames > 0) {
                int minNumPlayableFrames = Math.Min(prevNumPlayableFrames, m_playableKeyFrames.Count);
                for (int i = 0; i < minNumPlayableFrames; ++i) {
                    m_playableKeyFrames[i].SetKeyFrameMode(prevKeyFrameModes[i]);
                }
            }
        }
        
        //Refresh all markers
        double timePerFrame      = TimelineUtility.CalculateTimePerFrame(clipOwner);
        int    numPlayableFrames = m_playableKeyFrames.Count;
        for (int i = 0; i < numPlayableFrames; ++i) {
            m_playableKeyFrames[i].SetIndexAndLocalTime(i, i * timePerFrame);
            m_playableKeyFrames[i].RefreshMarker(m_keyFrameMarkersVisibility);
        }
        
    }        
    
    
//----------------------------------------------------------------------------------------------------------------------
    //may return null
    internal PlayableKeyFrame GetPlayableFrame(int index) {
        if (null == m_playableKeyFrames || index >= m_playableKeyFrames.Count)
            return null;
        
        Assert.IsTrue(null!=m_playableKeyFrames && index < m_playableKeyFrames.Count);
        return m_playableKeyFrames[index];
    }


//----------------------------------------------------------------------------------------------------------------------
    
    private void UpdatePlayableFramesSize(int reqPlayableFramesSize) {
        TimelineClip clipOwner = GetOwner();
        Assert.IsNotNull(clipOwner);

        double timePerFrame = TimelineUtility.CalculateTimePerFrame(clipOwner);
        //Resize m_playableFrames
        if (m_playableKeyFrames.Count < reqPlayableFramesSize) {
            int             numNewPlayableFrames = (reqPlayableFramesSize - m_playableKeyFrames.Count);
            List<PlayableKeyFrame> newPlayableFrames = new List<PlayableKeyFrame>(numNewPlayableFrames);           
            for (int i = m_playableKeyFrames.Count; i < reqPlayableFramesSize; ++i) {
                newPlayableFrames.Add(CreatePlayableFrame(this,i, timePerFrame));
            }            
            m_playableKeyFrames.AddRange(newPlayableFrames);                
        }

        if (m_playableKeyFrames.Count > reqPlayableFramesSize) {
            int numLastPlayableFrames = m_playableKeyFrames.Count;
            for (int i = reqPlayableFramesSize; i < numLastPlayableFrames; ++i) {
                PlayableKeyFrame curKeyFrame = m_playableKeyFrames[i];
                curKeyFrame?.Destroy();
            }            
            m_playableKeyFrames.RemoveRange(reqPlayableFramesSize, numLastPlayableFrames - reqPlayableFramesSize);
        }
            
        Assert.IsTrue(m_playableKeyFrames.Count == reqPlayableFramesSize);
           
        for (int i = 0; i < reqPlayableFramesSize; ++i) {
            PlayableKeyFrame curPlayableKeyFrame = m_playableKeyFrames[i];
            Assert.IsNotNull(curPlayableKeyFrame);                
            m_playableKeyFrames[i].SetIndexAndLocalTime(i, timePerFrame * i);
            
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

    private int LocalTimeToFrameIndex(double localTime, double clipDuration) {
        int numPlayableFrames = m_playableKeyFrames.Count;
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
    [SerializeField] private List<PlayableKeyFrame> m_playableKeyFrames;
    [SerializeField] [HideInInspector] private bool m_keyFrameMarkersRequested = true;

    [SerializeField] [HideInInspector] private float m_lastClipStartTimeOnRefresh = 0;
    [SerializeField] [HideInInspector] private float m_lastClipDurationOnRefresh = 0;

    
#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_keyframeControllerClipDataVersion = CUR_KEYFRAME_CONTROLLER_CLIP_DATA_VERSION;        
#pragma warning restore 414    
    
#if UNITY_EDITOR    
    private double m_timelineWidthPerFrame = Int16.MaxValue;
    private bool   m_forceShowFrameMarkers = false;
#endif

    private bool m_keyFrameMarkersVisibility      = true;
    private bool m_needToRefreshTimelineEditor = false;
    
    private const int    CUR_KEYFRAME_CONTROLLER_CLIP_DATA_VERSION = 1;
    
}


} //end namespace

