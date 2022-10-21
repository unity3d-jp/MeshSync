using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using JetBrains.Annotations;
using UnityEngine.SceneManagement;

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
    , ITimelineClipAsset, IPlayableBehaviour, ISerializationCallbackReceiver 
{

    SceneCachePlayableAsset() : base() {
        m_editorConfig = new SceneCachePlayableAssetEditorConfig();
    }
    
    internal void Init(bool updateClipDurationOnCreatePlayable) {
        m_updateClipDurationOnCreatePlayable = updateClipDurationOnCreatePlayable;
    } 
    
    public void OnBeforeSerialize() {
            
    }

    public void OnAfterDeserialize() {
        if (null == m_editorConfig) {
            m_editorConfig = new SceneCachePlayableAssetEditorConfig();
        }
    }
    
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
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
            scClipData.InitPlayableFrames();
        }
#endif        
        
        return ScriptPlayable<SceneCachePlayableBehaviour>.Create(graph, behaviour);
    }
    
    internal void InitKeyFrames() {
        SceneCacheClipData clipData = GetBoundClipData();
        if (null == clipData)
            return;
        clipData.InitPlayableFrames();
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

        SetClipCurveInEditor(clip, m_animationCurve);
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
    
    //import old data. [TODO-sin: 2022-3-24] Remove in 0.13.x
    internal void SetIsSceneCacheCurveExtracted(bool extracted) { m_isSceneCacheCurveExtracted = extracted; }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    internal void AddKeyFrame(double time) {
        SceneCacheClipData clipData = GetBoundClipData();
        if (null == clipData) //Null check. the data might not have been bound during recompile
            return;
        
        clipData.AddKeyFrame(time);
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
        
        RecreateClipCurveInEditor(clipData, sceneCacheInfo.GetNumFrames());
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
    
    static void RecreateClipCurveInEditor(SceneCacheClipData clipData, int numSceneCacheFrames) {

        int numPlayableKeyFrames = clipData.GetNumPlayableFrames();
        if (numPlayableKeyFrames <= 0 || numSceneCacheFrames <= 0) {
            AnimationCurve dummyCurve = new AnimationCurve();
            SetClipCurveInEditor(clipData.GetOwner(), dummyCurve);
            return;
        }
        
        List<Keyframe>   keys                  =  new List<Keyframe>();
        SISPlayableFrame firstPlayableKeyFrame = clipData.GetPlayableFrame(0);

        //always create curve key for the first keyframe
        KeyFrameMode firstCurveKeyMode = firstPlayableKeyFrame.IsEnabled() ? (KeyFrameMode) firstPlayableKeyFrame.GetProperty(KeyFramePropertyID.Mode) : KeyFrameMode.Hold;
        AddCurveKey(keys, (float)firstPlayableKeyFrame.GetLocalTime(), firstPlayableKeyFrame.GetPlayFrame(), numSceneCacheFrames, firstCurveKeyMode);
        
        for (int i = 1; i < numPlayableKeyFrames; ++i) {
            SISPlayableFrame curPlayableKeyFrame = clipData.GetPlayableFrame(i);
            if (!curPlayableKeyFrame.IsEnabled())
                continue;
            
            KeyFrameMode mode = (KeyFrameMode)(curPlayableKeyFrame.GetProperty(KeyFramePropertyID.Mode));            
            AddCurveKey(keys, (float) curPlayableKeyFrame.GetLocalTime(), curPlayableKeyFrame.GetPlayFrame(), numSceneCacheFrames, mode);            
        }
        
        AnimationCurve curve = new AnimationCurve(keys.ToArray());
        SetClipCurveInEditor(clipData.GetOwner(), curve);
    }
    
    static void RecreateClipCurveInEditorOld(SceneCacheClipData clipData, int numFrames) {

        int          numKeyFrames                     = clipData.GetNumPlayableFrames();
        Keyframe[]   keys                             = new Keyframe[numKeyFrames];
        float        lastEnabledKeyFrameAnimationTime = 0;
        bool         lastEnabledKeyFrameIsHold        = false;
        HashSet<int> keysToLinearize                  = new HashSet<int>();
        int          prevEnabledKey                   = 0;
        for (int i = 0; i < numKeyFrames; ++i) {
            SISPlayableFrame curKeyFrame = clipData.GetPlayableFrame(i);
            KeyFrameMode     mode        = (KeyFrameMode)(curKeyFrame.GetProperty(KeyFramePropertyID.Mode));
            
            float animationTime = Mathf.Clamp(((float) curKeyFrame.GetPlayFrame() / numFrames),0,1);

            keys[i].time = (float) curKeyFrame.GetLocalTime();
            
            keys[i].inTangent = keys[i].outTangent = float.PositiveInfinity;
            
            if (curKeyFrame.IsEnabled()) {
                lastEnabledKeyFrameAnimationTime = animationTime;
                lastEnabledKeyFrameIsHold        = mode == KeyFrameMode.Hold;

                keys[i].value = animationTime;
                
                LinearizeKeyValues(ref keys, prevEnabledKey, i, keysToLinearize);
                keysToLinearize.Clear();
                prevEnabledKey = i;
            } else {
                if (lastEnabledKeyFrameIsHold) {
                    keys[i].value = lastEnabledKeyFrameAnimationTime;
                } else {
                    keysToLinearize.Add(i);
                }
            }
        }
        
        //regard last disabled keys as hold 
        foreach(int index in keysToLinearize) {
            keys[index].value = lastEnabledKeyFrameAnimationTime;
        }
        
        AnimationCurve curve = new AnimationCurve(keys);
        SetClipCurveInEditor(clipData.GetOwner(), curve);
    }

    static void LinearizeKeyValues(ref Keyframe[] keys, int linearStartIndex, int linearEndIndex, HashSet<int> indicesToUpdate) {

        Keyframe startKey = keys[linearStartIndex];
        Keyframe endKey   = keys[linearEndIndex];
        
        AnimationCurve linearCurve = AnimationCurve.Linear(startKey.time, startKey.value, endKey.time, endKey.value);
        foreach (int index in indicesToUpdate) {
            keys[index].value = linearCurve.Evaluate(keys[index].time);
        }
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

    [HideInInspector][SerializeField] private bool m_isSceneCacheCurveExtracted = false;
 
    [HideInInspector][SerializeField] private SceneCachePlayableAssetEditorConfig m_editorConfig;
    
//----------------------------------------------------------------------------------------------------------------------

    private bool m_updateClipDurationOnCreatePlayable = false;
    
    private IExposedPropertyTable m_propertyTable;
    private SceneCachePlayer      m_sceneCachePlayer;
    private SceneCachePlayer      m_extractedSceneCachePlayer;

}


}