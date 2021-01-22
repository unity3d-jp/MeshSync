using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

[System.Serializable] 
internal class SceneCachePlayableAsset : BaseExtendedClipPlayableAsset<SceneCacheClipData>, ITimelineClipAsset {
    
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
    
    [SerializeField] private ExposedReference<SceneCachePlayer>  m_sceneCachePlayerRef;

    
    SceneCachePlayableBehaviour m_sceneCachePlayableBehaviour = null;
    
}


}