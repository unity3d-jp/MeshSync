using Unity.FilmInternalUtilities;
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
        
#pragma warning disable 612
        //import old data. [TODO-sin: 2022-3-24] Remove in 0.13.x
        scClipData.CopyLegacyClipDataToAsset(this);
#pragma warning restore 612
        
        
        m_propertyTable             = graph.GetResolver();
        m_sceneCachePlayer          = m_sceneCachePlayerRef.Resolve(m_propertyTable);
        m_extractedSceneCachePlayer = m_extractedSceneCachePlayerRef.Resolve(m_propertyTable);
        
        behaviour.SetSceneCachePlayer(m_sceneCachePlayer);
        behaviour.SetPlayableAsset(this);

            
        if (!m_sceneCachePlayer) {
            m_isSceneCacheCurveExtracted = false;
            return ScriptPlayable<SceneCachePlayableBehaviour>.Create(graph, behaviour);
        }

        m_sceneCachePlayer.SetAutoplay(false);
        
#if UNITY_EDITOR        
        //Initialize curve
        if (ShouldExtractCurveInEditor()) {
            TimelineClip clip = scClipData.GetOwner();
            Assert.IsNotNull(clip);
            float updatedCurveDuration = ExtractSceneCacheCurveInEditor(clip);
            if (m_updateClipDurationOnCreatePlayable) {
                clip.duration = updatedCurveDuration;
            }
            m_updateClipDurationOnCreatePlayable = false;
        }
#endif        
        
        return ScriptPlayable<SceneCachePlayableBehaviour>.Create(graph, behaviour);
    }

//----------------------------------------------------------------------------------------------------------------------
    
    #region IPlayableBehaviour interfaces
    /// <inheritdoc/>
    public void OnBehaviourPause(Playable playable, FrameData info) { }
    
    /// <inheritdoc/>
    public void OnBehaviourPlay(Playable playable, FrameData info) { }
    
    
    /// <inheritdoc/>
    public void OnGraphStart(Playable playable) { }
    
    
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
    
    private static AnimationCurve CreateLinearAnimationCurve(TimelineClip clip) {
        return AnimationCurve.Linear(0f, 0f,(float) (clip.duration * clip.timeScale), 1f );
    }
    

//----------------------------------------------------------------------------------------------------------------------

#if UNITY_EDITOR
   
        
    internal void SetCurveToLinearInEditor() {
        TimelineClip clip = GetBoundClipData()?.GetOwner();
        Assert.IsNotNull(clip);
       
        m_animationCurve = CreateLinearAnimationCurve(clip);
        UpdateClipCurveInEditor(clip, m_animationCurve);        
    }

    private static void UpdateClipCurveInEditor(TimelineClip clip, AnimationCurve animationCurveToApply) {
        
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

    internal void ApplyOriginalSceneCacheCurveInEditor() {
        if (null == m_sceneCachePlayer)
            return;

        TimelineClip clip = GetBoundClipData()?.GetOwner();
        Assert.IsNotNull(clip);
        ExtractSceneCacheCurveInEditor(clip);
    }    
    
    bool ShouldExtractCurveInEditor() {
        if (null == m_sceneCachePlayer) {
            return false;
        }
        
        //no need to update if the curve has been extracted
        if (m_isSceneCacheCurveExtracted && (m_extractedSceneCachePlayer == m_sceneCachePlayer)) {
            return false;
        }

        return true;
    }

    //returns curve duration
    private float ExtractSceneCacheCurveInEditor(TimelineClip clip) {
       
        //Bind for the first time
        m_animationCurve = ExtractNormalizedTimeCurve(m_sceneCachePlayer, out float updatedCurveDuration);
        if (null == m_animationCurve) {
            m_animationCurve     = CreateLinearAnimationCurve(clip);
            updatedCurveDuration = (float) clip.duration; //no change
        }

        UpdateClipCurveInEditor(clip, m_animationCurve);
        m_isSceneCacheCurveExtracted = true;
        SetExtractedSceneCachePlayerInEditor(m_sceneCachePlayer);
        return updatedCurveDuration;
    }    
    
    
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

    internal ExposedReference<SceneCachePlayer> GetSceneCachePlayerRef() { return m_sceneCachePlayerRef;}
    
    internal SceneCachePlayer GetSceneCachePlayer() => m_sceneCachePlayer;
    internal LimitedAnimationController GetOverrideLimitedAnimationController() => m_overrideLimitedAnimationController;     
    

    internal void           SetAnimationCurve(AnimationCurve curve) { m_animationCurve = curve; }
    internal AnimationCurve GetAnimationCurve()                     => m_animationCurve;
    
    
    //import old data. [TODO-sin: 2022-3-24] Remove in 0.13.x
    internal void SetIsSceneCacheCurveExtracted(bool extracted) { m_isSceneCacheCurveExtracted = extracted; }
    
#if UNITY_EDITOR
    
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
    
//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private ExposedReference<SceneCachePlayer> m_sceneCachePlayerRef;
    [HideInInspector][SerializeField] private ExposedReference<SceneCachePlayer> m_extractedSceneCachePlayerRef;
    
    [SerializeField] private LimitedAnimationController m_overrideLimitedAnimationController = new LimitedAnimationController();
    
    [SerializeField] private double      m_time;

    [HideInInspector][SerializeField] private AnimationCurve m_animationCurve = AnimationCurve.Constant(0,0,0);

    [HideInInspector][SerializeField] private bool m_isSceneCacheCurveExtracted = false;
    
//----------------------------------------------------------------------------------------------------------------------

    private bool m_updateClipDurationOnCreatePlayable = false;
    
    private IExposedPropertyTable m_propertyTable;
    private SceneCachePlayer      m_sceneCachePlayer;
    private SceneCachePlayer      m_extractedSceneCachePlayer;

}


}