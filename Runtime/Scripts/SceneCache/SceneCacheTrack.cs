using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEngine.UI;

namespace Unity.StreamingImageSequence {

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

    public override void GatherProperties(PlayableDirector director, IPropertyCollector driver) {
        Image ps = director.GetGenericBinding(this) as Image;
        if (ps == null) return;

        GameObject go = ps.gameObject;
        driver.AddFromName<Image>(go, "m_Color");

    }
}

}