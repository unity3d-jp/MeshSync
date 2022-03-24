﻿using System;
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
    protected override void OnBeforeSerializeInternalV() {
        m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION;
    }

    protected override void OnAfterDeserializeInternalV() {
        if (m_sceneCacheClipDataVersion == CUR_SCENE_CACHE_CLIP_DATA_VERSION) {
            return;
        }

        if (m_sceneCacheClipDataVersion < (int) SceneCacheClipDataVersion.MovedLimitedAnimationController_0_12_6) {
            m_copyLimitedAnimationControllerToAsset = true;
        }

        if (m_sceneCacheClipDataVersion < (int) SceneCacheClipDataVersion.MovedAnimationCurve_0_12_6) {
            m_copyAnimationCurveToAsset = true;
        }
        
        m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    internal override void DestroyV() { }
    
//----------------------------------------------------------------------------------------------------------------------
    
    //returns if updated or not
    internal bool BindSceneCachePlayer(SceneCachePlayer sceneCachePlayer, out float updatedCurveDuration) {
        //update data structure if clipData has already been flagged as initialized.
        //m_scPlayer can initially be null after deserializing.
        if (m_initialized && (null == m_scPlayer || sceneCachePlayer == m_scPlayer)) {
            m_scPlayer           = sceneCachePlayer;
            updatedCurveDuration = 0;
            return false;
        }

        TimelineClip clip = GetOwner();
        Assert.IsNotNull(clip);
       
        //Bind for the first time
        m_scPlayer       = sceneCachePlayer;
        m_animationCurve = ExtractNormalizedTimeCurve(m_scPlayer, out updatedCurveDuration);
        if (null == m_animationCurve) {
            m_animationCurve = CreateLinearAnimationCurve(clip);
            updatedCurveDuration    = (float) clip.duration;
        }

        UpdateClipCurve(clip, m_animationCurve);
        m_initialized = true;
        return true;
    }

    internal void UnbindSceneCachePlayer() {
        m_scPlayer    = null;
        m_initialized = false;
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
        
        AnimationCurve origCurve = ExtractNormalizedTimeCurve(m_scPlayer, out float _);
        if (null == origCurve) {
            Debug.LogWarning("Scene Cache doesn't have curve: " + m_scPlayer.gameObject.ToString());
            return;
        } 
        
        m_animationCurve = origCurve;
        UpdateClipCurve(clip, m_animationCurve);
    }
    
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
    
    private static AnimationCurve CreateLinearAnimationCurve(TimelineClip clip) {
        return AnimationCurve.Linear(0f, 0f,(float) (clip.duration * clip.timeScale), 1f );        
    }
    
    private static void UpdateClipCurve(TimelineClip clip, AnimationCurve animationCurveToApply) {

#if UNITY_EDITOR        
        
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

    //[TODO-sin:2022-3-24] remove this in 0.13.x
    internal void CopyLegacyClipDataToAsset(SceneCachePlayableAsset sceneCachePlayableAsset) {
        if (m_copyLimitedAnimationControllerToAsset) {
            LimitedAnimationController assetController = sceneCachePlayableAsset.GetOverrideLimitedAnimationController();
            assetController.SetEnabled(m_overrideLimitedAnimationController.IsEnabled());
            assetController.SetFrameOffset(m_overrideLimitedAnimationController.GetFrameOffset());
            assetController.SetNumFramesToHold(m_overrideLimitedAnimationController.GetNumFramesToHold());
            m_copyLimitedAnimationControllerToAsset = false;
        }


        if (m_copyAnimationCurveToAsset) {
            sceneCachePlayableAsset.SetAnimationCurve(m_animationCurve);
            sceneCachePlayableAsset.SetIsSceneCacheCurveExtracted(m_initialized);
            m_copyAnimationCurveToAsset = false;
        }
    }
//----------------------------------------------------------------------------------------------------------------------
   
    [SerializeField] private AnimationCurve m_animationCurve = AnimationCurve.Constant(0,0,0);
    [SerializeField] private bool           m_initialized    = false;

    [SerializeField] private LimitedAnimationController m_overrideLimitedAnimationController = new LimitedAnimationController();

#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION; 
#pragma warning restore 414    
    
//----------------------------------------------------------------------------------------------------------------------

    //Can't use SerializeField, because SceneCacheClipData is an asset which belongs to a track.  
    SceneCachePlayer m_scPlayer = null;
    
    private const int CUR_SCENE_CACHE_CLIP_DATA_VERSION = (int) SceneCacheClipDataVersion.MovedAnimationCurve_0_12_6;

    private bool m_copyLimitedAnimationControllerToAsset = false;
    private bool m_copyAnimationCurveToAsset = false;

//----------------------------------------------------------------------------------------------------------------------

    internal enum SceneCacheClipDataVersion {
        Initial = 1, 
        MovedLimitedAnimationController_0_12_6, //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        MovedAnimationCurve_0_12_6, //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        
    } 
    
}


} //end namespace


