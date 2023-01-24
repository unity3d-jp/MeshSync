using System.IO;
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

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    internal static SceneCachePlayer CreateSceneCachePlayer() {
        GameObject       sceneCacheGo     = new GameObject();
        SceneCachePlayer sceneCachePlayer = sceneCacheGo.AddComponent<SceneCachePlayer>();
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(sceneCachePlayer, Path.GetFullPath(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH));
        return sceneCachePlayer;
    }
    
}
} //end namespace