using JetBrains.Annotations;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor {

[CustomTimelineEditor(typeof(SceneCachePlayableAsset)), UsedImplicitly]
internal class SceneCachePlayableAssetEditor : ClipEditor {

    

    [InitializeOnLoadMethod]
    static void SceneCachePlayableAssetEditor_OnEditorLoad() {
        
        EditorSceneManager.sceneSaved += SceneCachePlayableAssetEditor_OnSceneSaved;
    }
    
    static void SceneCachePlayableAssetEditor_OnSceneSaved(Scene scene) {
        //Workaround to prevent errors: "The Playable is invalid. It has either been Disposed or was never created."
        //when editing curves after saving the scene
        TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved);    
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    /// <inheritdoc/>
    public override void OnCreate(TimelineClip clip, TrackAsset track, TimelineClip clonedFrom) {
        
        SceneCachePlayableAsset asset = clip.asset as SceneCachePlayableAsset;
        if (null == asset) {
            Debug.LogError("[MeshSync] Asset is not a SceneCachePlayableAsset: " + clip.asset);
            return;
        }

        //OnCreate() is called the clip is assigned to the track, but we need the track for creating curves.
        clip.parentTrack = track;
                       
        //If the clip already has curves (because of cloning, etc), then we don't set anything
        if (null == clip.curves) {
            CreateClipCurve(clip);
        }        
    }

//----------------------------------------------------------------------------------------------------------------------    
    //Called when a clip is changed by the Editor. (TrimStart, TrimEnd, etc)    
    public override void OnClipChanged(TimelineClip clip) {       
        base.OnClipChanged(clip);

        SceneCachePlayableAsset playableAsset = clip.asset as SceneCachePlayableAsset;
        if (null == playableAsset) {
            Debug.LogWarning("[MeshSync] Clip Internal Error: Asset is not SceneCache");
            return;            
        }
        
        SceneCacheClipData clipData = playableAsset.GetBoundClipData() as SceneCacheClipData;
        if (null == clipData) {
            //The clip is not ready. Not deserialized yet
            return;
        }
        
        Assert.IsNotNull(clip.curves);
               
        //Always apply clipCurves to clipData
        AnimationCurve curve = AnimationUtility.GetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding());        
        clipData.SetAnimationCurve(curve);
        
    }    

//----------------------------------------------------------------------------------------------------------------------

    private void CreateClipCurve(TimelineClip clip) {        
        clip.CreateCurves("Curves: " + clip.displayName);
        
        //Init dummy linear curve
        AnimationCurve curve = AnimationCurve.Linear(0f,0f,(float)clip.duration,1f);
        clip.curves.SetCurve("", typeof(SceneCachePlayableAsset), "m_time", curve);
        TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved );
        
        
    }

//----------------------------------------------------------------------------------------------------------------------    

}
} //end namespace