﻿using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using JetBrains.Annotations;

#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.Timeline;
#endif

namespace Unity.MeshSync {

/// <summary>
/// The PlayableAsset of the TimelineClip to be used inside the Timeline Window.
/// Implements the following interfaces:
/// - ITimelineClipAsset: for defining clip capabilities (ClipCaps) 
/// - IPlayableBehaviour: for displaying the curves in the timeline window
/// </summary>

[System.Serializable] 
internal class SceneCachePlayableAsset : BaseExtendedClipPlayableAsset<SceneCacheClipData>
    , ITimelineClipAsset, IPlayableBehaviour 
{

    internal void Init(bool updateClipDurationOnCreatePlayable) {
        m_updateClipDurationOnCreatePlayable = updateClipDurationOnCreatePlayable;
    } 
    
//----------------------------------------------------------------------------------------------------------------------
    public ClipCaps clipCaps {
        get { return ClipCaps.ClipIn | ClipCaps.SpeedMultiplier | ClipCaps.Extrapolation; }
    }

    public override Playable CreatePlayable(PlayableGraph graph, GameObject go) {
        
        SceneCachePlayableBehaviour behaviour = new SceneCachePlayableBehaviour();
        SceneCacheClipData scClipData = GetBoundClipData() as SceneCacheClipData;        
        Assert.IsNotNull(scClipData);
        
        m_propertyTable             = graph.GetResolver();
        m_sceneCachePlayer          = m_sceneCachePlayerRef.Resolve(m_propertyTable);
        m_extractedSceneCachePlayer = m_extractedSceneCachePlayerRef.Resolve(m_propertyTable);
        
        behaviour.SetSceneCachePlayer(m_sceneCachePlayer);
        behaviour.SetPlayableAsset(this);

            
        if (!m_sceneCachePlayer) {
            return ScriptPlayable<SceneCachePlayableBehaviour>.Create(graph, behaviour);
        }
        
#if UNITY_EDITOR        
        //Initialize duration
        if (m_updateClipDurationOnCreatePlayable) {
            TimelineClip clip = scClipData.GetOwner();
            Assert.IsNotNull(clip);
            float updatedCurveDuration = FindSceneCacheDurationInEditor(clip);
            
            m_updateClipDurationOnCreatePlayable = false;
            clip.duration                        = updatedCurveDuration;
        }
        scClipData.InitKeyFrames();
#endif
        
        return ScriptPlayable<SceneCachePlayableBehaviour>.Create(graph, behaviour);
    }
    
    internal void InitKeyFrames() {
        SceneCacheClipData clipData = GetBoundClipData();
        if (null == clipData)
            return;
        clipData.InitKeyFrames();
    }
    

//----------------------------------------------------------------------------------------------------------------------
    
    #region IPlayableBehaviour interfaces
    /// <inheritdoc/>
    public void OnBehaviourPause(Playable playable, FrameData info) { }
    
    /// <inheritdoc/>
    public void OnBehaviourPlay(Playable playable, FrameData info) { }


    /// <inheritdoc/>
    public void OnGraphStart(Playable playable) {
        SceneCacheClipData clipData = GetBoundClipData();
        clipData?.OnGraphStart(); //Null check. the data might not have been bound during recompile
        
    }
    
    
    /// <inheritdoc/>
    public void OnGraphStop(Playable playable) { }
    
    /// <inheritdoc/>
    public void OnPlayableCreate(Playable playable) { }
    
    /// <inheritdoc/>
    public void OnPlayableDestroy(Playable playable) { }
    
    /// <inheritdoc/>
    public void PrepareFrame(Playable playable, FrameData info) { }

    /// <inheritdoc/>
    public void ProcessFrame(Playable playable, FrameData info, object playerData) { }

    #endregion
    
//----------------------------------------------------------------------------------------------------------------------
    [CanBeNull]
    private static AnimationCurve ExtractNormalizedTimeCurve(SceneCachePlayer scPlayer, out float duration) {

        ISceneCacheInfo sceneCacheInfo = scPlayer.ExtractSceneCacheInfo(forceOpen:true);
        if (null == sceneCacheInfo) {
            duration = 0;
            return null;
        }

        TimeRange timeRange = sceneCacheInfo.GetTimeRange(); 
        duration = timeRange.GetDuration(); 
        if (duration <= 0f) {
            duration = Mathf.Epsilon;
        }

        Keyframe[] keyframes = sceneCacheInfo.GetTimeCurve().keys;
        int numKeyframes = keyframes.Length;
        for (int i = 0; i < numKeyframes; ++i) {
            keyframes[i].value /= timeRange.end;
        }
        
        //outTangent
        for (int i = 0; i < numKeyframes-1; ++i) {
            keyframes[i].outTangent = CalculateLinearTangent(keyframes, i, i+1);
        }
        
        //inTangent
        for (int i = 1; i < numKeyframes; ++i) {
            keyframes[i].inTangent = CalculateLinearTangent(keyframes, i-1, i);
        }
        
        AnimationCurve curve = new AnimationCurve(keyframes);
        return curve;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    private static float CalculateLinearTangent(Keyframe[] keyFrames, int index, int toIndex) {
        return (float) (((double) keyFrames[index].value - (double) keyFrames[toIndex].value) 
            / ((double) keyFrames[index].time - (double) keyFrames[toIndex].time));
    }

    private static float CalculateLinearTangent(Keyframe key0, Keyframe key1) {
        return (key0.value - key1.value) / (key0.time - key1.time);
    }
    
    private static AnimationCurve CreateLinearAnimationCurve(TimelineClip clip) {
        return AnimationCurve.Linear(0f, 0f,(float) (clip.duration * clip.timeScale), 1f );
    }
    

//----------------------------------------------------------------------------------------------------------------------

#if UNITY_EDITOR
   
        
    internal void SetCurveToLinearInEditor() {
        TimelineClip clip = GetBoundClipData()?.GetOwner();
        Assert.IsNotNull(clip);
       
        m_animationCurve = CreateLinearAnimationCurve(clip);
        SetClipCurveInEditor(clip, m_animationCurve);
    }

    private static void SetClipCurveInEditor(TimelineClip clip, AnimationCurve animationCurveToApply) {
        
        bool shouldRefresh = (null == clip.curves);
        
        if (!shouldRefresh) {
            AnimationCurve shownCurve = AnimationUtility.GetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding());
            shouldRefresh = !CurveApproximately(shownCurve, animationCurveToApply);
        } else {
            clip.CreateCurves("Curves: " + clip.displayName);
        }
        
        
        AnimationUtility.SetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding(),animationCurveToApply);
        
        if (shouldRefresh) {
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved );
        }
        
    }

    private float FindSceneCacheDurationInEditor(TimelineClip clip) {
       
        //Bind for the first time
        ISceneCacheInfo sceneCacheInfo = m_sceneCachePlayer.ExtractSceneCacheInfo(forceOpen:true);
        if (null == sceneCacheInfo) {
            return (float) clip.duration;
        }
        
        TimeRange timeRange = sceneCacheInfo.GetTimeRange(); 
        return timeRange.GetDuration();
    }    
    
    
    static bool CurveApproximately(AnimationCurve x, AnimationCurve y) {
        if (null == x && null == y)
            return true;
        
        if (null == x || null == y)
            return false;

        Keyframe[] xKeys = x.keys;
        Keyframe[] yKeys = y.keys;

        int numKeys0 = xKeys.Length; 

        if (numKeys0 != yKeys.Length)
            return false;

        if (0 == numKeys0)
            return true;
        
        HashSet<int> framesToCheck = new HashSet<int>() {
            0, 
            xKeys.Length - 1
        };

        foreach (int frame in framesToCheck) {
            if (!KeyframeApproximately(xKeys[frame], yKeys[frame]))
                return false;
        }

        return true;
    }

    static bool KeyframeApproximately(Keyframe k0, Keyframe k1) {
        //only check time and value
        return Mathf.Approximately(k0.time, k1.time) && Mathf.Approximately(k0.value, k1.value);
    }
    
#endif //UNITY_EDITOR    

//----------------------------------------------------------------------------------------------------------------------    

    internal ExposedReference<SceneCachePlayer> GetSceneCachePlayerRef() { return m_sceneCachePlayerRef;}
    
    internal SceneCachePlayer GetSceneCachePlayer() => m_sceneCachePlayer;
    internal LimitedAnimationController GetOverrideLimitedAnimationController() => m_overrideLimitedAnimationController;     
    

    internal void           SetAnimationCurve(AnimationCurve curve) { m_animationCurve = curve; }
    internal AnimationCurve GetAnimationCurve()                     => m_animationCurve;
    

    internal SceneCachePlayableAssetEditorConfig GetEditorConfig() { return m_editorConfig;}
        
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal void AddKeyFrame(double time) {
        SceneCacheClipData clipData = GetBoundClipData();        
        clipData?.AddKeyFrame(time); //Null check. the data might not have been bound during recompile
    }
    
#if UNITY_EDITOR
    
    
    internal void OnClipChanged() {
        SceneCacheClipData clipData = GetBoundClipData();
        if (null == clipData) //Null check. the data might not have been bound during recompile
            return;
        
        clipData.OnClipChanged();

        if (null == m_sceneCachePlayer)
            return;
        
        ISceneCacheInfo sceneCacheInfo = m_sceneCachePlayer.ExtractSceneCacheInfo(forceOpen:true);
        if (null == sceneCacheInfo) {
            return;
        }
        
        m_animationCurve = RecreateClipCurveInEditor(clipData, sceneCacheInfo);
    }

    static void AddCurveKey(List<Keyframe> keys, float time, int frameToPlay, int numFrames, KeyFrameMode mode) {
        Keyframe key = new Keyframe(time, Mathf.Clamp((float) frameToPlay / numFrames,0,1));
        switch (mode) {
            case KeyFrameMode.Hold: key.outTangent = float.PositiveInfinity;  break;
            default:                break;
        }

        int curNumKeys = keys.Count;
        if  (curNumKeys > 0) {
            Keyframe prevKey       = keys[curNumKeys - 1];
            bool     isPrevKeyHold = float.IsPositiveInfinity(prevKey.outTangent);
            if (isPrevKeyHold) {
                key.inTangent = float.PositiveInfinity;
            } else {
                prevKey.outTangent = CalculateLinearTangent(prevKey, key);
                key.inTangent      = prevKey.outTangent;
            }
            keys[curNumKeys - 1] = prevKey;
        }
        
        keys.Add(key);
    }
    
    static AnimationCurve RecreateClipCurveInEditor(SceneCacheClipData clipData, ISceneCacheInfo sceneCacheInfo) {
        AnimationCurve ret = new AnimationCurve();

        int numSceneCacheFrames = sceneCacheInfo.GetNumFrames();
        int numKeyFrames        = clipData.GetNumKeyFrames();
        
        if (numKeyFrames <= 0 || numSceneCacheFrames <= 0) {
            SetClipCurveInEditor(clipData.GetOwner(), ret);
            return ret;
        }
        
        List<Keyframe>   keys          = new List<Keyframe>();
        PlayableKeyFrame firstKeyFrame = clipData.GetKeyFrame(0);
        if (null == firstKeyFrame) {
            Debug.LogError($"[MeshSync] Internal errors in clip {clipData.GetOwner().displayName}");
            return ret;
        }

        //always create curve key for the first keyframe
        KeyFrameMode firstCurveKeyMode = firstKeyFrame.IsEnabled() ? firstKeyFrame.GetKeyFrameMode() : KeyFrameMode.Hold;
        AddCurveKey(keys, (float)firstKeyFrame.GetLocalTime(), firstKeyFrame.GetPlayFrame(), numSceneCacheFrames, firstCurveKeyMode);
        
        for (int i = 1; i < numKeyFrames; ++i) {
            PlayableKeyFrame curKeyFrame = clipData.GetKeyFrame(i);
            if (null == curKeyFrame)
                continue;
            if (!curKeyFrame.IsEnabled())
                continue;
            
            KeyFrameMode mode = curKeyFrame.GetKeyFrameMode();
            AddCurveKey(keys, (float) curKeyFrame.GetLocalTime(), curKeyFrame.GetPlayFrame(), numSceneCacheFrames, mode);
        }
        
        ret = new AnimationCurve(keys.ToArray());
        SetClipCurveInEditor(clipData.GetOwner(), ret);
        return ret;
    }
    
    internal void SetSceneCachePlayerInEditor(SceneCachePlayer scPlayer) {
        ExposedReferenceUtility.SetReferenceValueInEditor(ref m_sceneCachePlayerRef, m_propertyTable, scPlayer);
    }
    
    private void SetExtractedSceneCachePlayerInEditor(SceneCachePlayer scPlayer) {
        ExposedReferenceUtility.SetReferenceValueInEditor(ref m_extractedSceneCachePlayerRef, m_propertyTable, scPlayer);
    }

       
    internal static EditorCurveBinding GetTimeCurveBinding() {return m_timeCurveBinding; }
    
    private static EditorCurveBinding m_timeCurveBinding =  
        new EditorCurveBinding() {
            path         = "",
            type         = typeof(SceneCachePlayableAsset),
            propertyName = "m_time"
        };
#endif 
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private ExposedReference<SceneCachePlayer> m_sceneCachePlayerRef;
    [HideInInspector][SerializeField] private ExposedReference<SceneCachePlayer> m_extractedSceneCachePlayerRef;
    
    [SerializeField] private LimitedAnimationController m_overrideLimitedAnimationController = new LimitedAnimationController();
    
    [SerializeField] private double      m_time;

    [HideInInspector][SerializeField] private AnimationCurve m_animationCurve = AnimationCurve.Constant(0,0,0);
 
    [HideInInspector][SerializeField] private SceneCachePlayableAssetEditorConfig m_editorConfig = new SceneCachePlayableAssetEditorConfig();
    
//----------------------------------------------------------------------------------------------------------------------

    private bool m_updateClipDurationOnCreatePlayable = false;
    
    private IExposedPropertyTable m_propertyTable;
    private SceneCachePlayer      m_sceneCachePlayer;
    private SceneCachePlayer      m_extractedSceneCachePlayer;

}


}