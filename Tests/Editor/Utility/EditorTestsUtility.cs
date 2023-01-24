using System.IO;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor.Tests {
internal static class EditorTestsUtility {

    private static void InitTimelineTest(bool enableSceneCacheGo, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip) {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        director         = CreateDirector();
        sceneCachePlayer = CreateSceneCachePlayer();
        sceneCachePlayer.gameObject.SetActive(enableSceneCacheGo);
        clip = SceneCachePlayerEditorUtility.AddSceneCacheTrackAndClip(director, "TestSceneCacheTrack", sceneCachePlayer);

        TimelineEditorUtility.SelectDirectorInTimelineWindow(director); //trigger the TimelineWindow's update etc.
        TimelineEditorUtility.RefreshTimelineEditor();
    }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    internal static PlayableDirector CreateDirector() {
        PlayableDirector director = new GameObject("Director").AddComponent<PlayableDirector>();
        TimelineAsset    asset    = ScriptableObject.CreateInstance<TimelineAsset>();
        director.playableAsset = asset;
        return director;
    }

    
    internal static SceneCachePlayer CreateSceneCachePlayer() {
        GameObject       sceneCacheGo     = new GameObject();
        SceneCachePlayer sceneCachePlayer = sceneCacheGo.AddComponent<SceneCachePlayer>();
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(sceneCachePlayer, Path.GetFullPath(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH));
        return sceneCachePlayer;
    }
    
}
} //end namespace