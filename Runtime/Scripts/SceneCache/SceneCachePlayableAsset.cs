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
        
        SceneCachePlayableBehaviour bh = new SceneCachePlayableBehaviour();
        bh.SetSceneCachePlayer(m_sceneCachePlayerRef.Resolve(graph.GetResolver()));
        return ScriptPlayable<SceneCachePlayableBehaviour>.Create(graph, bh);
    }
   
//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private ExposedReference<SceneCachePlayer>  m_sceneCachePlayerRef = new ExposedReference<SceneCachePlayer>();

}


}