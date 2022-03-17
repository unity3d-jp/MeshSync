using JetBrains.Annotations;
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
        
        //OnCreate() is called before the clip is assigned to the track, but we need the track for creating curves.
        clip.TryMoveToTrack(track);
                       
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
        
        //Check if the curves is null, which may happen if the clip is created using code ?
        if (null == clip.curves) {
            CreateClipCurve(clip);
        }        
        
        SceneCacheClipData clipData = playableAsset.GetBoundClipData() as SceneCacheClipData;
        if (null == clipData) {
            //The clip is not ready. Not deserialized yet
            return;
        }
        
               
        //Always apply clipCurves to clipData
        AnimationCurve curve = AnimationUtility.GetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding());        
        clipData.SetAnimationCurve(curve);
        
    }    

//----------------------------------------------------------------------------------------------------------------------
    
    /// <inheritdoc/>
    public override void DrawBackground(TimelineClip clip, ClipBackgroundRegion region) {
        base.DrawBackground(clip, region);
        
        SceneCachePlayableAsset asset = clip.asset as SceneCachePlayableAsset;
        if (null == asset) {
            Debug.LogError("Asset is not a SceneCachePlayableAsset: " + clip.asset);
            return;
        }
        
        SceneCacheClipData clipData = asset.GetBoundClipData();
        if (null == clipData)
            return;

        LimitedAnimationController limitedAnimationController = clipData.GetOverrideLimitedAnimationController();
        if (!limitedAnimationController.IsEnabled()) {
            return;
        }

        int numFrames = limitedAnimationController.GetNumFramesToHold();
        int offset    = limitedAnimationController.GetFrameOffset();
            
        GUIStyle style = new GUIStyle(GUI.skin.label) {
            alignment = TextAnchor.LowerRight,
            normal    = {
                textColor = Color.green
            }
        };
        GUIContent laContent = new GUIContent($"Limited On: {numFrames}, {offset}");
        
        Vector2 laContentSize = style.CalcSize(laContent);
        Rect rect = region.position;
        if (rect.width <= laContentSize.x)
            return;
        
        EditorGUI.LabelField(rect, laContent, style);

        
    }
    
    
//----------------------------------------------------------------------------------------------------------------------

    private void CreateClipCurve(TimelineClip clip) {        
        clip.CreateCurves("Curves: " + clip.displayName);
        
        //Init dummy linear curve
        AnimationCurve curve = AnimationCurve.Linear(0f,0f,(float)clip.duration,1f);
        AnimationUtility.SetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding(),curve);
        TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved );
        
        
    }

//----------------------------------------------------------------------------------------------------------------------    

}
} //end namespace