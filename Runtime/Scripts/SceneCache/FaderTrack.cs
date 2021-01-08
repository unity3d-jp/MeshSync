using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEngine.UI;

namespace Unity.StreamingImageSequence {

[TrackClipType(typeof(FaderPlayableAsset))]
[TrackBindingType(typeof(Image))]
[TrackColor(0.263f, 0.09f, 0.263f)]
[NotKeyable]
internal class FaderTrack : TrackAsset {
    public override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount) {

        ScriptPlayable<FaderPlayableMixer> mixerScriptPlayable = ScriptPlayable<FaderPlayableMixer>.Create(graph, inputCount);
        PlayableDirector director = go.GetComponent<PlayableDirector>();
        Assert.IsNotNull(director);
        
        //Initialize mixer
        Image image = director.GetGenericBinding(this) as Image;           
        FaderPlayableMixer mixer = mixerScriptPlayable.GetBehaviour();
        mixer.Init(null==image ? null : image.gameObject, director, GetClips());
        
        
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