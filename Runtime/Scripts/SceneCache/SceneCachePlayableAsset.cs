using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;

#if UNITY_EDITOR
using UnityEditor;
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
        
        SceneCacheClipData scClipData = GetBoundClipData() as SceneCacheClipData;        
        Assert.IsNotNull(scClipData);
        
#pragma warning disable 612
        //import old data. [TODO-sin: 2022-3-24] Remove in 0.13.x
        scClipData.CopyLegacyClipDataToAsset(this);
#pragma warning restore 612
        
        
        m_propertyTable = graph.GetResolver();
        m_sceneCachePlayer = m_sceneCachePlayerRef.Resolve(graph.GetResolver());

        if (!m_sceneCachePlayer) {
            scClipData.UnbindSceneCachePlayer(); //clear curve
            return Playable.Create(graph);
        }
        
        //Initialize curve
        m_sceneCachePlayer.SetAutoplay(false);
        bool updated = scClipData.BindSceneCachePlayer(m_sceneCachePlayer, out float updatedCurveDuration);
        if (m_updateClipDurationOnCreatePlayable && updated) {
            scClipData.GetOwner().duration = updatedCurveDuration;
        }
        
        m_updateClipDurationOnCreatePlayable = false;
        return Playable.Create(graph);
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

    internal ExposedReference<SceneCachePlayer> GetSceneCachePlayerRef() { return m_sceneCachePlayerRef;}
    
    internal LimitedAnimationController GetOverrideLimitedAnimationController() => m_overrideLimitedAnimationController;     
    

#if UNITY_EDITOR
    
    internal void SetSceneCachePlayerInEditor(SceneCachePlayer scPlayer) {
        //check if exposedName hasn't been initialized
        if (m_sceneCachePlayerRef.exposedName.ToString() == ":0") {
            m_sceneCachePlayerRef.exposedName = GUID.Generate().ToString();
        }
        m_propertyTable.SetReferenceValue(m_sceneCachePlayerRef.exposedName, scPlayer);
    }
    internal SceneCachePlayer GetSceneCachePlayer() => m_sceneCachePlayer;
    
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
    [SerializeField] private LimitedAnimationController m_overrideLimitedAnimationController = new LimitedAnimationController();
    
    [SerializeField] private double      m_time;

    [HideInInspector][SerializeField] private AnimationCurve m_animationCurve = AnimationCurve.Constant(0,0,0);
    
//----------------------------------------------------------------------------------------------------------------------

    private bool m_updateClipDurationOnCreatePlayable = false;
    
    private IExposedPropertyTable m_propertyTable;
    private SceneCachePlayer      m_sceneCachePlayer;
    
}


}