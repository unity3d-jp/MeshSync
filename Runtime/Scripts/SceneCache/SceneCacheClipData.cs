using System;
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
internal class SceneCacheClipData : BaseClipData {

//----------------------------------------------------------------------------------------------------------------------
    
    internal void BindSceneCachePlayer(SceneCachePlayer sceneCachePlayer) {
        if (sceneCachePlayer == m_scPlayer) {
            return;
        }
        
        

        m_scPlayer         = sceneCachePlayer;
        m_animationCurve = ExtractNormalizedTimeCurve(m_scPlayer);
        if (null == m_animationCurve) {
            ResetAnimationCurve();
        }
        UpdateClipAnimationCurve();
        
    }

    internal void UnbindSceneCachePlayer() {        
        m_scPlayer = null;        
        ResetAnimationCurve();
        UpdateClipAnimationCurve();
    }

//----------------------------------------------------------------------------------------------------------------------
    [CanBeNull]
    private static AnimationCurve ExtractNormalizedTimeCurve(SceneCachePlayer scPlayer) {
        AnimationCurve origTimeCurve = scPlayer.GetTimeCurve();
        TimeRange timeRange = scPlayer.GetTimeRange();

        if (null == origTimeCurve) {
            return null;
        }

        float timeLength = timeRange.end - timeRange.start;
        if (Mathf.Approximately(0, timeLength)) {
            timeLength = Mathf.Epsilon;
        }

        Keyframe[] keyframes = origTimeCurve.keys;
        int numKeyframes = keyframes.Length;
        for (int i = 0; i < numKeyframes; ++i) {
            keyframes[i].value /= timeLength;
        }        
        
        AnimationCurve curve = new AnimationCurve(keyframes);        
        return curve;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    private void ResetAnimationCurve() {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
        m_animationCurve = AnimationCurve.Linear(0f, 0f,(float) clip.duration, 1f );        
    }
    

    
    void UpdateClipAnimationCurve() {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);

        bool shouldRefresh = false;
        
#if UNITY_EDITOR        
        AnimationCurve shownCurve = AnimationUtility.GetEditorCurve(clip.curves, m_timeCurveBinding);
        shouldRefresh = !CurveApproximately(shownCurve, m_animationCurve); 
#endif
        
        clip.curves.SetCurve("", typeof(SceneCachePlayableAsset), "m_time", m_animationCurve);
        
#if UNITY_EDITOR        
        if (shouldRefresh) {
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved );            
        }
#endif
        
        
    }

//----------------------------------------------------------------------------------------------------------------------
    static bool CurveApproximately(AnimationCurve x, AnimationCurve y) {
        if (null == x && null == y)
            return true;
        
        if (null == x || null == y)
            return false;

        Keyframe[] xKeys = x.keys;
        Keyframe[] yKeys = y.keys;

        if (xKeys.Length != yKeys.Length)
            return false;

        int lastIndex = xKeys.Length-1;
        
        //only check first and last frame
        if ( KeyframeApproximately(xKeys[0],yKeys[0]) && KeyframeApproximately(xKeys[lastIndex],yKeys[lastIndex])) {
            return true;
        }

        return false;

    }

    static bool KeyframeApproximately(Keyframe k0, Keyframe k1) {
        //only check time and value
        return Mathf.Approximately(k0.time, k1.time) && Mathf.Approximately(k0.value, k1.value);
    }
    
    

//----------------------------------------------------------------------------------------------------------------------
    internal void           SetAnimationCurve(AnimationCurve curve) { m_animationCurve = curve; }
    internal AnimationCurve GetAnimationCurve()                     {  return m_animationCurve; }
        
//----------------------------------------------------------------------------------------------------------------------
   
    [SerializeField] private SceneCachePlayer m_scPlayer;
   
    
//----------------------------------------------------------------------------------------------------------------------

    [SerializeField] private AnimationCurve m_animationCurve;


    private static EditorCurveBinding m_timeCurveBinding =  
        new EditorCurveBinding() {
            path         = "",
            type         = typeof(SceneCachePlayableAsset),
            propertyName = "m_time"
        };    
}


} //end namespace


