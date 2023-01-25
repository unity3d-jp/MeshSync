using System.Collections;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine.TestTools;
using UnityEngine.Timeline;
using UnityEditor.Timeline;

namespace Unity.MeshSync.Editor.Tests {

internal class SceneCacheKeyFrameTests {
    
    [UnityTest]
    public IEnumerator ShowKeyFrameMarkers() {
        
        EditorTestsUtility.InitTimelineTest(enableSceneCacheGo:true, out _, out _, out TimelineClip clip);
        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(sceneCachePlayableAsset);
        yield return YieldEditorUtility.WaitForFramesAndIncrementUndo(1);
        
        //Show
        SceneCacheClipData clipData = sceneCachePlayableAsset.GetBoundClipData();
        Assert.IsNotNull(clipData);
        
        TrackAsset trackAsset = clip.GetParentTrack();
        clipData.RequestKeyFrameMarkers(true, true);
        TimelineEditor.Refresh(RefreshReason.ContentsModified);
        yield return YieldEditorUtility.WaitForFramesAndIncrementUndo(1);
        
        Assert.AreEqual(TimelineUtility.CalculateNumFrames(clip), trackAsset.GetMarkerCount());
        yield return YieldEditorUtility.WaitForFramesAndIncrementUndo(1);

        //Undo showing FrameMarkers
        EditorTestsUtility.UndoAndRefreshTimelineEditor();
        yield return YieldEditorUtility.WaitForFramesAndIncrementUndo(1);
        
        Assert.False(clipData.AreKeyFrameMarkersRequested());
        Assert.AreEqual(0, trackAsset.GetMarkerCount());        
        yield return YieldEditorUtility.WaitForFramesAndIncrementUndo(1);
    }
    
    
}

} //end namespace
