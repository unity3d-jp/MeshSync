﻿using JetBrains.Annotations;
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
            clip.CreateCurves("Curves: " + clip.displayName);
        }
        
        AnimationCurve animationCurve = AnimationCurve.Linear(0, 0, 1,1 ); 
        
        clip.curves.SetCurve("", typeof(SceneCachePlayableAsset), "m_time", animationCurve);
        TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved );
        
    }


}
} //end namespace