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
        return Playable.Create(graph);
    }

//----------------------------------------------------------------------------------------------------------------------
   
    
    [SerializeField] private ExposedReference<SceneCachePlayer>  m_sceneCachePlayer = new ExposedReference<SceneCachePlayer>();

//----------------------------------------------------------------------------------------------------------------------

}


}