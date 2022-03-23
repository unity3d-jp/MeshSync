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

    internal void Init(bool wasCloned) {
        m_wasCloned = wasCloned;
    } 
    
//----------------------------------------------------------------------------------------------------------------------
    public ClipCaps clipCaps {
        get { return ClipCaps.ClipIn | ClipCaps.SpeedMultiplier | ClipCaps.Extrapolation; }
    }

    public override Playable CreatePlayable(PlayableGraph graph, GameObject go) {
        
        SceneCacheClipData scClipData = GetBoundClipData() as SceneCacheClipData;        
        Assert.IsNotNull(scClipData);        
        
        SceneCachePlayer scPlayer = m_sceneCachePlayerRef.Resolve(graph.GetResolver());

        if (m_wasCloned) {
            scClipData.SetInitialized(true);
            m_wasCloned = false;
        }
        
        
        
        //Initialize or clear curve
        if (scPlayer) {
            scPlayer.SetAutoplay(false);
            scClipData.BindSceneCachePlayer(scPlayer);
        } else {
            scClipData.UnbindSceneCachePlayer();
        }

        return Playable.Create(graph);        
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


    private bool m_wasCloned = false;

}


}