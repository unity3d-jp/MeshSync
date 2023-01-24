using System.Collections;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using UnityEngine.TestTools;
using UnityEngine.Timeline;
using UnityEditor.Timeline;

namespace Unity.MeshSync.Editor.Tests {

internal class SceneCacheKeyFrameTests {
    
    [UnityTest]
    public IEnumerator ShowKeyFrameMarkers() {
        
#if !UNITY_2022_2_OR_NEWER
        EditorTestsUtility.InitTimelineTest(enableSceneCacheGo:true, out _, out _, out TimelineClip clip);
        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        Assert.IsNotNull(sceneCachePlayableAsset);
        yield return null;
        
        //Show
        SceneCacheClipData clipData = sceneCachePlayableAsset.GetBoundClipData();
        Assert.IsNotNull(clipData);
        
        TrackAsset trackAsset = clip.GetParentTrack();
        clipData.RequestKeyFrameMarkers(true, true);
        TimelineEditor.Refresh(RefreshReason.ContentsModified);
        yield return null;
        
        Assert.AreEqual(TimelineUtility.CalculateNumFrames(clip), trackAsset.GetMarkerCount());
        yield return null;


        //Undo showing FrameMarkers
        EditorTestsUtility.UndoAndRefreshTimelineEditor(); yield return null;
        Assert.False(clipData.AreKeyFrameMarkersRequested());
        Assert.AreEqual(0, trackAsset.GetMarkerCount());        
#endif
        yield return null;
    }
    
    
}

} //end namespace
