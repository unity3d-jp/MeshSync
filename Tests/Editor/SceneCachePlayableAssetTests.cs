using System;
using NUnit.Framework;
using System.Collections;
using System.IO;
using JetBrains.Annotations;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
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

        InitTest(true, out PlayableDirector _, out SceneCachePlayer _, out TimelineClip clip);
        yield return null;
        
        AnimationCurve curve = VerifyAnimationCurve(clip);
        Assert.IsNotNull(curve);
    }

    [UnityTest]
    public IEnumerator CreatePlayableAssetUsingDisabledGameObject() {

        InitTest(false, out PlayableDirector _, out SceneCachePlayer _, out TimelineClip clip);
        yield return null;
        
        AnimationCurve curve = VerifyAnimationCurve(clip);
        Assert.Greater(curve.keys.Length,2);
        Assert.Greater(clip.duration,0);
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    [UnityTest]
    public IEnumerator SetTimeInTimelineWindow() {

        InitTest(true, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip);
        yield return null;

        director.time = 0;
        yield return null;
                
        double timePerFrame = TimelineUtility.CalculateTimePerFrame(clip); 
        double directorTime = clip.start + clip.duration - timePerFrame;
        SetDirectorTime(director, directorTime); //this will trigger change in the time of the SceneCachePlayable
        yield return null;

        //Check clipData and curve
        AnimationCurve curve = VerifyAnimationCurve(clip);
        float normalizedTime = curve.Evaluate((float)directorTime);
                
        Assert.IsTrue(Mathf.Approximately((float)(normalizedTime * clip.duration), sceneCachePlayer.GetTime()));
    }

//----------------------------------------------------------------------------------------------------------------------    
    [UnityTest]
    public IEnumerator CheckGameObjectActiveStateInExtrapolatedClip() {

        InitTest(true, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip);
        yield return null;

        Assert.AreEqual(TimelineClip.ClipExtrapolation.None, clip.preExtrapolationMode);
        Assert.AreEqual(TimelineClip.ClipExtrapolation.None, clip.postExtrapolationMode);
        
        director.time = 0;
        yield return null;

        clip.start = 1.0f;
        GameObject scGameObject = sceneCachePlayer.gameObject;
        
        
        //start
        SetDirectorTime(director, 1.0f);
        yield return null;
        Assert.IsTrue(scGameObject.activeSelf);

        //before clip
        SetDirectorTime(director, 0.0f);
        yield return null;
        Assert.IsFalse(scGameObject.activeSelf);

        //half
        SetDirectorTime(director, clip.start + (clip.duration * 0.5f));
        yield return null;
        Assert.IsTrue(scGameObject.activeSelf);

        //after clip
        SetDirectorTime(director, clip.start + clip.duration + 1.0f);
        yield return null;
        Assert.IsFalse(scGameObject.activeSelf);
    }

//----------------------------------------------------------------------------------------------------------------------
    [UnityTest]
    public IEnumerator EnsureMatchingFramesAreLoadedToScene() {

        InitTest(true, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip);
        yield return null;

        yield return IterateAllSceneCacheFrames(director, clip, sceneCachePlayer, (int timelineFrame) => {
            Assert.AreEqual(timelineFrame, sceneCachePlayer.GetFrame());            
        });
        
    }

//----------------------------------------------------------------------------------------------------------------------
    [UnityTest]
    public IEnumerator EnsureLimitedFramesAreLoadedToScene() {

        InitTest(true, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip);
        yield return null;

        //Setup Limited Animation
        const int NUM_FRAMES_TO_HOLD = 3;
        const int OFFSET = 1;
        
        SceneCacheClipData         clipData                   = VerifyClipData(clip);
        LimitedAnimationController limitedAnimationController = clipData.GetOverrideLimitedAnimationController();
        limitedAnimationController.Enable(NUM_FRAMES_TO_HOLD,OFFSET);

        yield return IterateAllSceneCacheFrames(director, clip, sceneCachePlayer, (int timelineFrame) => {
            int shownFrame = sceneCachePlayer.GetFrame();
            Assert.Zero(shownFrame % NUM_FRAMES_TO_HOLD - OFFSET);            
        });

    }

//----------------------------------------------------------------------------------------------------------------------
    [UnityTest]
    public IEnumerator EnsureMatchingAndLimitedFramesAreLoadedToScene() {

        InitTest(true, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip0);
        yield return null;

        SceneCacheInfo scInfo = sceneCachePlayer.ExtractSceneCacheInfo(forceOpen:true);
        Assert.IsNotNull(scInfo);
        int    numFrames    = scInfo.numFrames;
        double timePerFrame = 1.0f / scInfo.sampleRate;

        //Setup clip0 duration
        int halfFrames = Mathf.FloorToInt(numFrames * 0.5f);
        clip0.duration = halfFrames * timePerFrame;  
        
        //Setup clip1 and its limited animation
        //TimelineClip clip1 = SceneCachePlayerEditorUtility.AddSceneCacheTrackAndClip(director, "TestSceneCacheTrack1", sceneCachePlayer);
        
        // TimelineAsset timelineAsset = director.playableAsset as TimelineAsset;
        // Assert.IsNotNull(timelineAsset);
        //
        // timelineAsset.CreateTrack<SceneCacheTrack>(null, "a");
        // timelineAsset.CreateTrack<SceneCacheTrack>(null, "b");
        // timelineAsset.CreateTrack<SceneCacheTrack>(null, "b");
        // timelineAsset.CreateTrack<SceneCacheTrack>(null, "c");
        
        
       TimelineClip  clip1         = clip0.GetParentTrack().CreateClip<SceneCachePlayableAsset>();
       clip1.start = clip0.start + clip0.duration + 0.1f;
       //  SceneCachePlayableAsset playableAsset1 = clip1.asset as SceneCachePlayableAsset;
       //  Assert.IsNotNull(playableAsset1);
       //  director.SetReferenceValue(playableAsset1.GetSceneCachePlayerRef().exposedName, sceneCachePlayer );
        
        //Force timeline to refresh
        // Selection.activeObject = sceneCachePlayer.gameObject;
        // yield return EditorTestsUtility.WaitForFrames(3);

        //
        
        TimelineEditorUtility.RefreshTimelineEditor( RefreshReason.ContentsAddedOrRemoved | RefreshReason.WindowNeedsRedraw | RefreshReason.ContentsModified);
            //        TimelineEditorUtility.SelectDirectorInTimelineWindow(director);
        yield return EditorTestsUtility.WaitForFrames(100);
        director.time = 1;
        yield return null;
        yield return null;
        
        //Setup Limited Animation
        // const int NUM_FRAMES_TO_HOLD = 3;
        // const int OFFSET             = 1;
        //
        // SceneCacheClipData         clipData1                   = VerifyClipData(clip1);
        // LimitedAnimationController limitedAnimationController = clipData1.GetOverrideLimitedAnimationController();
        // limitedAnimationController.Enable(NUM_FRAMES_TO_HOLD,OFFSET);
        //
        // clip1.clipIn   = clip0.duration;
        // clip1.duration = clip0.duration;        


        IEnumerator iterator0 = IterateAllSceneCacheFrames(director, clip0, sceneCachePlayer, (int frame) => {
            Assert.AreEqual(frame, sceneCachePlayer.GetFrame());
        });
        
        // IEnumerator iterator1 = IterateAllSceneCacheFrames(director, clip1, scData, (int frame) => {
        //     int shownFrame = sceneCachePlayer.GetFrame();
        //     Assert.Zero(shownFrame % NUM_FRAMES_TO_HOLD - OFFSET);            
        // });

        while (iterator0.MoveNext()) { yield return null; }        
//        while (iterator1.MoveNext()) { yield return null; }
        
    }
    
    
//----------------------------------------------------------------------------------------------------------------------

    private static void InitTest(bool enableSceneCacheGo, out PlayableDirector director, 
        out SceneCachePlayer sceneCachePlayer, out TimelineClip clip) 
    {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        director = CreateTestDirector();
        sceneCachePlayer = CreateTestSceneCachePlayer();
        sceneCachePlayer.gameObject.SetActive(enableSceneCacheGo);
        clip = SceneCachePlayerEditorUtility.AddSceneCacheTrackAndClip(director, "TestSceneCacheTrack", sceneCachePlayer);
        
        TimelineEditorUtility.SelectDirectorInTimelineWindow(director); //trigger the TimelineWindow's update etc.        
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

    [NotNull]
    private static AnimationCurve VerifyAnimationCurve(TimelineClip clip) {
        SceneCacheClipData clipData = VerifyClipData(clip);
        AnimationCurve curve = clipData.GetAnimationCurve();
        Assert.IsNotNull(curve);
        return curve;
    }

    [NotNull]
    private static SceneCacheClipData VerifyClipData(TimelineClip clip) {
        SceneCachePlayableAsset playableAsset = clip.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(playableAsset);
        SceneCacheClipData clipData = playableAsset.GetBoundClipData();
        Assert.IsNotNull(clipData);
        return clipData;
    }
    
    private IEnumerator IterateAllSceneCacheFrames(PlayableDirector director, TimelineClip clip, SceneCachePlayer scPlayer, 
        Action<int> afterUpdateFunc) 
    {
        ISceneCacheInfo scInfo = scPlayer.ExtractSceneCacheInfo(forceOpen:true);
        Assert.IsNotNull(scInfo);
        
        double timePerFrame = 1.0f / scInfo.GetSampleRate();
        
        //Use (numFrames-1) because when it becomes invisible when Timeline reaches the last frame
        for(int i=0;i<scInfo.GetNumFrames()-1;++i) {
            double elapsedTime = i * timePerFrame;
            if (elapsedTime >= clip.duration)
                yield break;
            
            double directorTime = clip.start + i * timePerFrame;
            SetDirectorTime(director, directorTime); //this will trigger change in the time of the SceneCachePlayable
            yield return null;

            afterUpdateFunc(i);
        }
        
    }    
    
//----------------------------------------------------------------------------------------------------------------------
    private static void SetDirectorTime(PlayableDirector director, double time) {
        director.time = time;
        TimelineEditor.Refresh(RefreshReason.SceneNeedsUpdate); 
    }        
    
}

} //end namespace
