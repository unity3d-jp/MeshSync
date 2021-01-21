using JetBrains.Annotations;
using Unity.FilmInternalUtilities;
using UnityEditor;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor {

[CustomTimelineEditor(typeof(SceneCachePlayableAsset)), UsedImplicitly]
internal class FaderPlayableAssetEditor : ClipEditor {

    /// <inheritdoc/>
    public override void OnCreate(TimelineClip clip, TrackAsset track, TimelineClip clonedFrom) {
        
        SceneCachePlayableAsset asset = clip.asset as SceneCachePlayableAsset;
        if (null == asset) {
            Debug.LogError("[MeshSync] Asset is not a SceneCachePlayableAsset: " + clip.asset);
            return;
        }
                       
        //If the clip already has curves (because of cloning, etc), then we don't set anything
        if (null == clip.curves) {
            InitCurve(clip);
        }
        TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved );
        
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
            clipData = playableAsset.BindNewClipData<SceneCacheClipData>(clip);
        }

        if (null == clip.curves) {
            InitCurve(clip);
        }
        
        
        AnimationCurve curve = AnimationUtility.GetEditorCurve(clip.curves, m_timeCurveBinding);
        clipData.SetAnimationCurve(curve);
    }    

//----------------------------------------------------------------------------------------------------------------------

    private void InitCurve(TimelineClip clip) {
        clip.CreateCurves("Curves: " + clip.displayName);
        AnimationCurve animationCurve = AnimationCurve.Linear(0, 0,3,1 ); 
        clip.curves.SetCurve("", typeof(SceneCachePlayableAsset), "m_time", animationCurve);            
        
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    private static EditorCurveBinding m_timeCurveBinding =  
        new EditorCurveBinding() {
            path         = "",
            type         = typeof(SceneCachePlayableAsset),
            propertyName = "m_time"
        };
    

}
} //end namespace