using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

[TrackClipType(typeof(SceneCachePlayableAsset))]
[TrackColor(0.196f, 0.804f, 0.196f)] //limegreen
internal class SceneCacheTrack : BaseExtendedClipTrack<SceneCacheClipData> {

    protected override Playable CreateTrackMixerInternal(PlayableGraph graph, GameObject go, int inputCount) {
        
        return ScriptPlayable<SceneCachePlayableMixer>.Create(graph, inputCount);
        
    }

}

}