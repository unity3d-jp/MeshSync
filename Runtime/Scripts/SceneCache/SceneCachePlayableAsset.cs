﻿using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;

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
        get {
            return ClipCaps.None;
        }
    }

    public override Playable CreatePlayable(PlayableGraph graph, GameObject go) {
        
        m_sceneCachePlayableBehaviour = new SceneCachePlayableBehaviour();
        SceneCacheClipData scClipData = GetBoundClipData() as SceneCacheClipData;        
        Assert.IsNotNull(scClipData);        
        
        SceneCachePlayer scPlayer = m_sceneCachePlayerRef.Resolve(graph.GetResolver());
        m_sceneCachePlayableBehaviour.SetSceneCachePlayer(scPlayer);
        m_sceneCachePlayableBehaviour.SetClipData(scClipData);
        
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

    internal float GetNormalizedTime() {
        if (null == m_sceneCachePlayableBehaviour) {
            return 0;            
        }

        SceneCachePlayer sc = m_sceneCachePlayableBehaviour.GetSceneCachePlayer();
        if (null == sc)
            return 0;
        
        return sc.GetRequestedNormalizedTime();
    }
    
//----------------------------------------------------------------------------------------------------------------------

    internal ExposedReference<SceneCachePlayer> GetSceneCachePlayerRef() { return m_sceneCachePlayerRef;}
    
//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private ExposedReference<SceneCachePlayer> m_sceneCachePlayerRef;
    [SerializeField]         double                             m_time;

    
    SceneCachePlayableBehaviour m_sceneCachePlayableBehaviour = null;
   
}


}