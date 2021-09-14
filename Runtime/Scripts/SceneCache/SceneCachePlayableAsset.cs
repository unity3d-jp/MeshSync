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
internal class SceneCachePlayableAsset : BaseExtendedClipPlayableAsset<SceneCacheClipData>, ITimelineClipAsset, IPlayableBehaviour {
    
//----------------------------------------------------------------------------------------------------------------------
    public ClipCaps clipCaps {
#if AT_USE_TIMELINE_GE_1_4_0            
        get { return ClipCaps.ClipIn | ClipCaps.AutoScale; }
#else            
        get { return ClipCaps.ClipIn | ClipCaps.SpeedMultiplier; }
#endif           
    }

    public override Playable CreatePlayable(PlayableGraph graph, GameObject go) {
        
        m_sceneCachePlayableBehaviour = new SceneCachePlayableBehaviour();
        SceneCacheClipData scClipData = GetBoundClipData() as SceneCacheClipData;        
        Assert.IsNotNull(scClipData);        
        
        SceneCachePlayer scPlayer = m_sceneCachePlayerRef.Resolve(graph.GetResolver());
        m_sceneCachePlayableBehaviour.SetSceneCachePlayer(scPlayer);
        m_sceneCachePlayableBehaviour.SetClipData(scClipData);
        m_sceneCachePlayableBehaviour.SetSnapToFrame(m_snapToFrame);
        
        //Initialize or clear curve
        if (scPlayer) {
            scPlayer.SetAutoplay(false);
            scClipData.BindSceneCachePlayer(scPlayer);
        } else {
            scClipData.UnbindSceneCachePlayer();
        }
        
        return ScriptPlayable<SceneCachePlayableBehaviour>.Create(graph, m_sceneCachePlayableBehaviour);
    }
   
//----------------------------------------------------------------------------------------------------------------------
    
    #region IPlayableBehaviour interfaces
    /// <inheritdoc/>
    public void OnBehaviourPause(Playable playable, FrameData info) {

    }
    
    /// <inheritdoc/>
    public void OnBehaviourPlay(Playable playable, FrameData info) {

    }
    
    
    /// <inheritdoc/>
    public void OnGraphStart(Playable playable) {       
    }
    
    
    /// <inheritdoc/>
    public void OnGraphStop(Playable playable) {
    }
    
    /// <inheritdoc/>
    public void OnPlayableCreate(Playable playable) {

    }
    /// <inheritdoc/>
    public void OnPlayableDestroy(Playable playable) {
    }
    
    /// <inheritdoc/>
    public void PrepareFrame(Playable playable, FrameData info) {

    }

    /// <inheritdoc/>
    public void ProcessFrame(Playable playable, FrameData info, object playerData) {
    }

    #endregion
    
    
//----------------------------------------------------------------------------------------------------------------------

    internal ExposedReference<SceneCachePlayer> GetSceneCachePlayerRef() { return m_sceneCachePlayerRef;}

    internal void SetSnapToFrame(SnapToFrame snap) { m_snapToFrame = snap;}
    internal SnapToFrame GetSnapToFrame() { return m_snapToFrame; }


#if UNITY_EDITOR    
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
    
    [SerializeField] private double      m_time;
    [SerializeField] private SnapToFrame m_snapToFrame = SnapToFrame.NEAREST;

    
    SceneCachePlayableBehaviour m_sceneCachePlayableBehaviour = null;
   
}


}