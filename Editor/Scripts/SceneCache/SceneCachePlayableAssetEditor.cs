using JetBrains.Annotations;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.Timeline;
using Unity.FilmInternalUtilities; //Required for TryMoveToTrack() in Timeline 1.4 and earlier 


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
    public override ClipDrawOptions GetClipOptions(TimelineClip clip) {
        ClipDrawOptions         clipOptions = base.GetClipOptions(clip);
        SceneCachePlayableAsset asset       = clip.asset as SceneCachePlayableAsset;
        if (null == asset) {
            Debug.LogError("Asset is not a SceneCachePlayableAsset: " + clip.asset);
            return clipOptions;
        }
        
        SceneCacheClipData clipData = asset.GetBoundClipData();
        if (null == clipData) 
            return clipOptions;

        SceneCachePlayer scPlayer = asset.GetSceneCachePlayer();
        if (null == scPlayer) {
            clipOptions.errorText = NO_SCENE_CACHE_ASSIGNED_ERROR;
            return clipOptions;
        }

        LimitedAnimationController overrideLimitedAnimationController =asset.GetOverrideLimitedAnimationController();
        
        if (!scPlayer.IsLimitedAnimationOverrideable() && overrideLimitedAnimationController.IsEnabled()) {
            clipOptions.errorText = UNABLE_TO_OVERRIDE_LIMITED_ANIMATION_ERROR;
            return clipOptions;
        }

        clipOptions.tooltip = scPlayer.GetSceneCacheFilePath();

        return clipOptions;
    } 
       
//----------------------------------------------------------------------------------------------------------------------    
    /// <inheritdoc/>
    public override void OnCreate(TimelineClip clip, TrackAsset track, TimelineClip clonedFrom) {
                
        SceneCachePlayableAsset asset = clip.asset as SceneCachePlayableAsset;
        if (null == asset) {
            Debug.LogError("[MeshSync] Asset is not a SceneCachePlayableAsset: " + clip.asset);
            return;
        }
        
        //Track can be null during copy and paste
        if (null != track) {
            //This callback occurs before the clip is assigned to the track, but we need the track for creating curves.
            clip.TryMoveToTrack(track);
        }

        asset.Init(updateClipDurationOnCreatePlayable: null == clonedFrom);
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
        
        //Always apply clipCurves to clipData
        AnimationCurve curve = AnimationUtility.GetEditorCurve(clip.curves, SceneCachePlayableAsset.GetTimeCurveBinding());        
        playableAsset.SetAnimationCurve(curve);
        
        playableAsset.RefreshPlayableFrames();
        
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

        LimitedAnimationController limitedAnimationController = asset.GetOverrideLimitedAnimationController();
        if (!limitedAnimationController.IsEnabled()) {
            return;
        }

        int numFrames = limitedAnimationController.GetNumFramesToHold();
        int offset    = limitedAnimationController.GetFrameOffset();
            
        GUIStyle style = new GUIStyle(GUI.skin.label) {
            alignment = TextAnchor.LowerRight,
            normal    = {
                textColor = new Color(0.3f,0.9f,0.3f),
            }
        };
        GUIContent laContent = new GUIContent($"Limited: {numFrames}, {offset}");
        
        Vector2 laContentSize = style.CalcSize(laContent);
        Rect rect = region.position;
        if (rect.width <= laContentSize.x * 2) //2: arbitrary
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
    
    private const string NO_SCENE_CACHE_ASSIGNED_ERROR              = "No Scene Cache Assigned";
    private const string UNABLE_TO_OVERRIDE_LIMITED_ANIMATION_ERROR = "Unable to override Limited Animation";
    
}
} //end namespace