using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor.Tests {
internal static class EditorTestsUtility {
    internal static PlayableDirector CreateDirector() {
        PlayableDirector director = new GameObject("Director").AddComponent<PlayableDirector>();
        TimelineAsset    asset    = ScriptableObject.CreateInstance<TimelineAsset>();
        director.playableAsset = asset;
        return director;
    }
}
} //end namespace