using System;
using JetBrains.Annotations;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;
using System.Collections.Generic; //HashSet

#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.Timeline;
#endif


namespace Unity.MeshSync {

[Serializable]
internal class SceneCacheClipData : BaseClipData {

//----------------------------------------------------------------------------------------------------------------------
    
    internal void BindSceneCachePlayer(SceneCachePlayer sceneCachePlayer) {
        if (sceneCachePlayer == m_scPlayer) {
            return;
        }

        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
       
        //Bind for the first time
        m_scPlayer         = sceneCachePlayer;
        m_animationCurve = ExtractNormalizedTimeCurve(m_scPlayer, out float endTime);
        if (null != m_animationCurve) {
            clip.duration = endTime;
        } else {
            m_animationCurve = CreateLinearAnimationCurve(clip);
            
        }
        UpdateClipAnimationCurve(clip, m_animationCurve);
        
    }

    internal void UnbindSceneCachePlayer() {        
        m_scPlayer = null;        
        
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
        
        m_animationCurve = CreateLinearAnimationCurve(clip);
        UpdateClipAnimationCurve(clip, m_animationCurve);
    }

//----------------------------------------------------------------------------------------------------------------------
    [CanBeNull]
    private static AnimationCurve ExtractNormalizedTimeCurve(SceneCachePlayer scPlayer, out float endTime) {
        AnimationCurve origTimeCurve = scPlayer.GetTimeCurve();

        if (null == origTimeCurve) {
            endTime = 0;
            return null;
        }

        TimeRange timeRange = scPlayer.GetTimeRange();
        endTime = timeRange.end;
        if (endTime <= 0f) {
            endTime = Mathf.Epsilon;
        }

        Keyframe[] keyframes = origTimeCurve.keys;
        int numKeyframes = keyframes.Length;
        for (int i = 0; i < numKeyframes; ++i) {
            keyframes[i].value /= endTime;
        }        
        
        AnimationCurve curve = new AnimationCurve(keyframes);        
        return curve;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    private static AnimationCurve CreateLinearAnimationCurve(TimelineClip clip) {
        return AnimationCurve.Linear(0f, 0f,(float) clip.duration, 1f );        
    }
    

    
    private static void UpdateClipAnimationCurve(TimelineClip clip, AnimationCurve animationCurveToApply) {

        bool shouldRefresh = false;
        
#if UNITY_EDITOR        
        AnimationCurve shownCurve = AnimationUtility.GetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding());
        shouldRefresh = !CurveApproximately(shownCurve, animationCurveToApply); 
#endif
        
        clip.curves.SetCurve("", typeof(SceneCachePlayableAsset), "m_time", animationCurveToApply);
        
#if UNITY_EDITOR        
        if (shouldRefresh) {
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved );            
        }
#endif
        
        
    }

//----------------------------------------------------------------------------------------------------------------------

#if UNITY_EDITOR
    static bool CurveApproximately(AnimationCurve x, AnimationCurve y) {
        if (null == x && null == y)
            return true;
        
        if (null == x || null == y)
            return false;

        Keyframe[] xKeys = x.keys;
        Keyframe[] yKeys = y.keys;

        if (xKeys.Length != yKeys.Length)
            return false;

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
    internal void           SetAnimationCurve(AnimationCurve curve) { m_animationCurve = curve; }
    internal AnimationCurve GetAnimationCurve()                     {  return m_animationCurve; }
        
//----------------------------------------------------------------------------------------------------------------------
   
    [SerializeField] private SceneCachePlayer m_scPlayer;
   
    
//----------------------------------------------------------------------------------------------------------------------

    [SerializeField] private AnimationCurve m_animationCurve;
    
}


} //end namespace


