﻿using NUnit.Framework;
using System.Collections;
using System.IO;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.Playables;
using UnityEditor.Timeline;
using UnityEngine.TestTools;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor.Tests {

internal class SceneCachePlayableAssetTests {

    [UnityTest]
    public IEnumerator CreatePlayableAsset() {

        InitTest(out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip);

        //Selecting director in Timeline Window will trigger the TimelineWindow's update etc.
        TimelineEditorUtility.SelectDirectorInTimelineWindow(director);
        yield return null;

        director.time = 0;
        yield return null;
        
        Assert.AreEqual(0, sceneCachePlayer.GetTime());
        SceneCacheTrack sceneCacheTrack = clip.GetParentTrack() as SceneCacheTrack;
        Assert.IsNotNull(sceneCacheTrack);
        double timePerFrame = 1.0f / sceneCacheTrack.timelineAsset.editorSettings.GetFPS();

        double directorTime = clip.start + clip.duration - timePerFrame;
        SetDirectorTime(director, directorTime); //this will trigger change in the time of the SceneCachePlayable
        yield return null;

        //Check clipData and curve
        SceneCachePlayableAsset playableAsset = clip.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(playableAsset);
        SceneCacheClipData clipData = playableAsset.GetBoundClipData();
        Assert.IsNotNull(clipData);        
        AnimationCurve curve = clipData.GetAnimationCurve();
        Assert.IsNotNull(curve);
        float normalizedTime = curve.Evaluate((float)directorTime);
                
        Assert.IsTrue(Mathf.Approximately((float)(normalizedTime * clip.duration), sceneCachePlayer.GetTime()));
    }


//----------------------------------------------------------------------------------------------------------------------

    private static void InitTest(out PlayableDirector director, 
        out SceneCachePlayer sceneCachePlayer, out TimelineClip clip) 
    {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        director = CreateTestDirector();
        sceneCachePlayer = CreateTestSceneCachePlayer();
        clip = SceneCachePlayerEditorUtility.AddSceneCacheTrackAndClip(director, "TestSceneCacheTrack", sceneCachePlayer);        
    }
    
    private static SceneCachePlayer CreateTestSceneCachePlayer() {
        GameObject       sceneCacheGo     = new GameObject();
        SceneCachePlayer sceneCachePlayer = sceneCacheGo.AddComponent<SceneCachePlayer>();
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(sceneCachePlayer, Path.GetFullPath(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH));
        return sceneCachePlayer;
    }

    private static PlayableDirector CreateTestDirector() {
        PlayableDirector director = new GameObject("Director").AddComponent<PlayableDirector>();
        TimelineAsset    asset    = ScriptableObject.CreateInstance<TimelineAsset>();
        director.playableAsset = asset;
        return director;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    private static void SetDirectorTime(PlayableDirector director, double time) {
        director.time = time;
        TimelineEditor.Refresh(RefreshReason.SceneNeedsUpdate); 
    }        
    
}

} //end namespace
