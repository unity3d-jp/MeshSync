using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

[TrackClipType(typeof(SceneCachePlayableAsset))]
[TrackColor(0.196f, 0.804f, 0.196f)] //limegreen
internal class SceneCacheTrack : BaseExtendedClipTrack<SceneCacheClipData> {

    protected override Playable CreateTrackMixerInternal(PlayableGraph graph, GameObject go, int inputCount) {
        
        
        ScriptPlayable<SceneCachePlayableMixer> playable = ScriptPlayable<SceneCachePlayableMixer>.Create(graph, inputCount);
        PlayableDirector director = go.GetComponent<PlayableDirector>();
        m_trackMixer = playable.GetBehaviour();
        
        if (null == director)
            return playable;


        m_trackMixer.Init(director, GetClips());

        return playable;
    }
    
    private void OnDestroy() {
        m_trackMixer?.Destroy();
        m_trackMixer = null;
    }


//----------------------------------------------------------------------------------------------------------------------    
    
    private SceneCachePlayableMixer m_trackMixer = null;   
    
}

}