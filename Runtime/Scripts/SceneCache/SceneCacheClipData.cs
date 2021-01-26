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
        
        if (m_initialized) {
            if (null == m_scPlayer) {
                //Assuming that BindSceneCachePlayer() will be called() during "deserialization", we initialize scPlayer
                //at first if this clipData has already been initialized.
                //SceneCachePlayer can't be deserialized as usual, because SceneCacheClip belongs to a track, which is an asset.
                m_scPlayer = sceneCachePlayer;
                return;
            } else if (m_scPlayer == sceneCachePlayer) {
                return;
            }
        }

        m_scPlayer    = sceneCachePlayer;
        m_initialized = true;
        

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

        ResetClip(clip);
        UpdateClipCurve(clip, m_animationCurve);
        
    }

    internal void UnbindSceneCachePlayer() {
        if (null == m_scPlayer)
            return;
        
        m_scPlayer    = null;
        m_initialized = false;
        
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
        
        m_animationCurve = CreateLinearAnimationCurve(clip);        
        ResetClip(clip);        
        UpdateClipCurve(clip, m_animationCurve);
    }

//----------------------------------------------------------------------------------------------------------------------
    
    internal void SetCurveToLinear() {
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
        clip.clipIn = 0;
        
        m_animationCurve = CreateLinearAnimationCurve(clip);
        UpdateClipCurve(clip, m_animationCurve);        
    }

    internal void ApplyOriginalSceneCacheCurve() {
        if (null == m_scPlayer)
            return;
                
        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
        clip.clipIn = 0;
        
        m_animationCurve = ExtractNormalizedTimeCurve(m_scPlayer, out float endTime);
        UpdateClipCurve(clip, m_animationCurve);
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
        return AnimationCurve.Linear(0f, 0f,(float) (clip.duration * clip.timeScale), 1f );        
    }


    private static void ResetClip(TimelineClip clip) {
        clip.clipIn    = 0;
        clip.timeScale = 1;
        
    }
    
    private static void UpdateClipCurve(TimelineClip clip, AnimationCurve animationCurveToApply) {

#if UNITY_EDITOR        
        
        bool shouldRefresh = false;
        
        AnimationCurve shownCurve = AnimationUtility.GetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding());
        shouldRefresh = !CurveApproximately(shownCurve, animationCurveToApply); 
        
        clip.curves.SetCurve("", typeof(SceneCachePlayableAsset), "m_time", animationCurveToApply);
        
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
   
    [SerializeField] private AnimationCurve   m_animationCurve;
    [SerializeField] private bool             m_initialized = false;

//----------------------------------------------------------------------------------------------------------------------
    
    SceneCachePlayer m_scPlayer    = null;
    
}


} //end namespace


