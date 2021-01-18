using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

[System.Serializable] 
internal class SceneCachePlayableAsset : PlayableAsset, ITimelineClipAsset {
    
//----------------------------------------------------------------------------------------------------------------------
    public ClipCaps clipCaps {
        get {
            return ClipCaps.None;
        }
    }

    public override Playable CreatePlayable(PlayableGraph graph, GameObject go) {
        
        m_sceneCachePlayableBehaviour = new SceneCachePlayableBehaviour();
        m_sceneCachePlayableBehaviour.SetSceneCachePlayer(m_sceneCachePlayerRef.Resolve(graph.GetResolver()));
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
        
        return sc.GetNormalizedTime();
    }

//----------------------------------------------------------------------------------------------------------------------

    internal ExposedReference<SceneCachePlayer> GetSceneCachePlayerRef() { return m_sceneCachePlayerRef;}
    
//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private ExposedReference<SceneCachePlayer>  m_sceneCachePlayerRef;

    
    SceneCachePlayableBehaviour m_sceneCachePlayableBehaviour = null;
    
}


}