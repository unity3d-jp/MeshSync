using NUnit.Framework;
using System.Collections;
using System.IO;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.Playables;
using UnityEditor;
using UnityEditor.Timeline;
using UnityEngine.TestTools;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor.Tests {

internal class SceneCachePlayableAssetTest {

    [UnityTest]
    public IEnumerator CreatePlayableAsset() {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        //Add director
        GameObject directorGo = new GameObject("Director");
        PlayableDirector director = directorGo.AddComponent<PlayableDirector>();
        
        //Setup scene cache            
        GameObject       sceneCacheGo     = new GameObject();
        SceneCachePlayer sceneCachePlayer = sceneCacheGo.AddComponent<SceneCachePlayer>();
        Assert.IsFalse(sceneCachePlayer.IsSceneCacheOpened());       
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(sceneCachePlayer, Path.GetFullPath(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH));
        

        //Create timeline asset
        TimelineAsset asset = ScriptableObject.CreateInstance<TimelineAsset>();
        director.playableAsset = asset;

        //Create PlayableAsset/Track/ etc
        SceneCachePlayableAsset playableAsset   = ScriptableObject.CreateInstance<SceneCachePlayableAsset>();
        SceneCacheTrack         sceneCacheTrack = asset.CreateTrack<SceneCacheTrack>(null, "Footage");
        TimelineClip            clip       = sceneCacheTrack.CreateDefaultClip();
        clip.asset = playableAsset;        
        director.SetReferenceValue(playableAsset.GetSceneCachePlayerRef().exposedName, sceneCachePlayer );


        //Select gameObject and open Timeline Window. This will trigger the TimelineWindow's update etc.
        EditorApplication.ExecuteMenuItem("Window/Sequencing/Timeline");
        Selection.activeTransform = directorGo.transform;
        yield return null;

        director.time = 0;
        yield return null;
        
        Assert.AreEqual(0, playableAsset.GetNormalizedTime());
        director.time = clip.duration-0.0001f;
        TimelineEditor.Refresh(RefreshReason.SceneNeedsUpdate);         
        yield return null;
        
        Assert.AreEqual(1, playableAsset.GetNormalizedTime());
    }
   
    
}

} //end namespace
