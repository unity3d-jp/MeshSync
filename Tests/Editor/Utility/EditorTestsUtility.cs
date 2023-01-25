using System.IO;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor.Tests {
internal static class EditorTestsUtility {

    internal static void InitTimelineTest(bool enableSceneCacheGo, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip) {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        director         = CreateTestDirector();
        sceneCachePlayer = CreateTestSceneCachePlayer();
        sceneCachePlayer.gameObject.SetActive(enableSceneCacheGo);
        clip = SceneCachePlayerEditorUtility.AddSceneCacheTrackAndClip(director, "TestSceneCacheTrack", sceneCachePlayer);

        TimelineEditorUtility.SelectDirectorInTimelineWindow(director); //trigger the TimelineWindow's update etc.
        TimelineEditorUtility.RefreshTimelineEditor();
    }
    
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //[TODO-sin: 2023-1-14] Move to FIU
    internal static void UndoAndRefreshTimelineEditor() {
        Undo.PerformUndo(); 
        TimelineEditor.Refresh(RefreshReason.ContentsModified);
    }
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    private static PlayableDirector CreateTestDirector() {
        PlayableDirector director = new GameObject("Director").AddComponent<PlayableDirector>();
        TimelineAsset    asset    = ScriptableObject.CreateInstance<TimelineAsset>();
        director.playableAsset = asset;
        return director;
    }


    private static SceneCachePlayer CreateTestSceneCachePlayer() {
        GameObject       sceneCacheGo     = new GameObject();
        SceneCachePlayer sceneCachePlayer = sceneCacheGo.AddComponent<SceneCachePlayer>();
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(sceneCachePlayer, Path.GetFullPath(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH));
        return sceneCachePlayer;
    }
    
}
} //end namespace