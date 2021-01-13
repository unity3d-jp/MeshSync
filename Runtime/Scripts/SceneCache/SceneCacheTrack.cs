using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEngine.UI;

namespace Unity.MeshSync {

[TrackClipType(typeof(SceneCachePlayableAsset))]
[TrackColor(0.263f, 0.09f, 0.263f)]
[NotKeyable]
internal class SceneCacheTrack : TrackAsset {
    public override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount) {

        ScriptPlayable<SceneCachePlayableMixer> mixerScriptPlayable = ScriptPlayable<SceneCachePlayableMixer>.Create(graph, inputCount);
        PlayableDirector director = go.GetComponent<PlayableDirector>();
        Assert.IsNotNull(director);
        
        //Initialize mixer
        SceneCachePlayableMixer mixer = mixerScriptPlayable.GetBehaviour();
        mixer.Init(director, GetClips());
        
        
        return mixerScriptPlayable;
    }
}

}