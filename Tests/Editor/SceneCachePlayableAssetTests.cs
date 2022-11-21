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

        InitTest(false, out PlayableDirector director, out SceneCachePlayer _, out TimelineClip clip);
        yield return null;
        
        SetDirectorTime(director, 0);
        yield return null;
        
        AnimationCurve curve = VerifyAnimationCurve(clip);        
        Assert.Greater(curve.keys.Length,2);
        Assert.Greater(clip.duration,0);
    }

//----------------------------------------------------------------------------------------------------------------------    
    [UnityTest]
    public IEnumerator ChangeSceneCache() {

        InitTest(true, out PlayableDirector _, out SceneCachePlayer scPlayer, out TimelineClip clip);
        yield return null;
        
        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(sceneCachePlayableAsset);

        sceneCachePlayableAsset.SetSceneCachePlayerInEditor(null);
        double halfDuration = clip.duration * 0.5f;
        clip.duration = halfDuration;
        TimelineEditor.Refresh(RefreshReason.ContentsModified);
        yield return null;
        
        sceneCachePlayableAsset.SetSceneCachePlayerInEditor(scPlayer);
        TimelineEditor.Refresh(RefreshReason.ContentsModified);
        yield return null;
        
        Assert.IsTrue(Mathf.Approximately((float)halfDuration, (float)clip.duration),"Clip Duration has been reset.");
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    [UnityTest]
    public IEnumerator SetTimeToHalfClipDuration() {

        InitTest(true, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip);
        yield return null;
        
        ISceneCacheInfo scInfo = sceneCachePlayer.ExtractSceneCacheInfo(forceOpen: true);
        Assert.IsNotNull(scInfo);
        float halfDuration = Mathf.FloorToInt(scInfo.GetNumFrames() * 0.5f) / scInfo.GetSampleRate();


        clip.start    = halfDuration;
        director.time = 0;
        yield return null;
                
        double directorTime = clip.start + halfDuration;
        SetDirectorTime(director, directorTime); //this will trigger change in the time of the SceneCachePlayable
        yield return null;
        TimelineEditor.Refresh(RefreshReason.SceneNeedsUpdate);
        yield return null;

        float scTime = sceneCachePlayer.GetTime();
        Assert.IsTrue(Mathf.Approximately((float)(halfDuration), scTime), $"Time: {scTime}. Expected: {halfDuration}");
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
    public IEnumerator CheckGameObjectActiveStateReferredInMultipleTracks() {

        InitTest(true, out PlayableDirector director, out SceneCachePlayer sceneCachePlayer, out TimelineClip clip0);
        yield return null;
        
        TimelineClip clip1 = SceneCachePlayerEditorUtility.AddSceneCacheTrackAndClip(director, "TestSceneCacheTrack 1", sceneCachePlayer);
        clip1.start = clip0.end;
        TimelineEditorUtility.RefreshTimelineEditor( RefreshReason.ContentsAddedOrRemoved );
        yield return null;
       
        GameObject scGameObject = sceneCachePlayer.gameObject;
        
        SetDirectorTime(director, clip0.start);
        yield return null;
        Assert.IsTrue(scGameObject.activeSelf);
        
        SetDirectorTime(director, clip1.start + clip1.duration * 0.5f);
        yield return null;
        Assert.IsTrue(scGameObject.activeSelf);
        
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
        
        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(sceneCachePlayableAsset);
        LimitedAnimationController limitedAnimationController = sceneCachePlayableAsset.GetOverrideLimitedAnimationController();
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

        
        ISceneCacheInfo scInfo = sceneCachePlayer.ExtractSceneCacheInfo(forceOpen:true);
        Assert.IsNotNull(scInfo);
        int    numFrames    = scInfo.GetNumFrames();
        double timePerFrame = 1.0f / scInfo.GetSampleRate();

        //Setup clip0 duration
        int halfFrames = Mathf.FloorToInt(numFrames * 0.5f);
        clip0.duration = halfFrames * timePerFrame;  
        
        //Setup clip1         
        TimelineClip clip1 = clip0.GetParentTrack().CreateClip<SceneCachePlayableAsset>();
        clip1.start    = clip0.start + clip0.duration;
        clip1.clipIn   = clip0.duration;
        clip1.duration = clip0.duration;
        SceneCachePlayableAsset playableAsset1 = clip1.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(playableAsset1);
        director.SetReferenceValue(playableAsset1.GetSceneCachePlayerRef().exposedName, sceneCachePlayer );
        
        TimelineEditorUtility.RefreshTimelineEditor( RefreshReason.ContentsAddedOrRemoved | RefreshReason.WindowNeedsRedraw | RefreshReason.ContentsModified);
        yield return null;
        
        //Setup Limited Animation
        const int NUM_FRAMES_TO_HOLD = 3;
        const int OFFSET             = 1;
        
        LimitedAnimationController limitedAnimationController1 = playableAsset1.GetOverrideLimitedAnimationController();
        limitedAnimationController1.Enable(NUM_FRAMES_TO_HOLD,OFFSET);
        
        yield return IterateAllSceneCacheFrames(director, clip0, sceneCachePlayer, (int timelineFrame) => {
            EditorApplication.isPaused = true;
            Assert.AreEqual(timelineFrame, sceneCachePlayer.GetFrame());
        });
        
        yield return IterateAllSceneCacheFrames(director, clip1, sceneCachePlayer, (int timelineFrame) => {
            int shownFrame = sceneCachePlayer.GetFrame();
            if (shownFrame == (scInfo.GetNumFrames() - 1)) //clamped to the end frame
                return;
                
            Assert.Zero(shownFrame % NUM_FRAMES_TO_HOLD - OFFSET);
        });
        
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
        TimelineEditorUtility.RefreshTimelineEditor();
        
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
        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(sceneCachePlayableAsset);
        AnimationCurve curve = sceneCachePlayableAsset.GetAnimationCurve();
        Assert.IsNotNull(curve);
        return curve;
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
