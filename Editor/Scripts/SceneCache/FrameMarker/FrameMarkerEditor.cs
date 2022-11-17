﻿using System.Globalization;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor {

[CustomTimelineEditor(typeof(SceneCacheFrameMarker))]
class FrameMarkerEditor : MarkerEditor {

    public override void DrawOverlay(IMarker m, MarkerUIStates uiState, MarkerOverlayRegion region) {
        SceneCacheFrameMarker marker = m as SceneCacheFrameMarker;
        if (null == marker)
            return;

        PlayableFrame playableFrame = marker.GetOwner();
        
        //Check invalid PlayableFrame/ClipData. Perhaps because of unsupported Duplicate operation ?
        PlayableFrameClipData clipData = playableFrame?.GetOwner();
        if (clipData == null)
            return;
        
        KeyFrameMode keyFrameMode = playableFrame.GetKeyFrameMode();

        const int TEXT_WIDTH = 30;
        Rect      labelRect  = region.markerRegion;
        labelRect.x     += labelRect.width;
        labelRect.width =  TEXT_WIDTH;
        DrawFrameNumber(labelRect, clipData, playableFrame.GetPlayFrame());
        
        switch (keyFrameMode) {
            case KeyFrameMode.Continuous: {
                UnityEngine.Graphics.DrawTexture(region.markerRegion, EditorTextures.GetKeyFrameSmoothTexture());
                break;
            }
            case KeyFrameMode.Hold: {
                UnityEngine.Graphics.DrawTexture(region.markerRegion, EditorTextures.GetKeyFrameStopTexture());
                break;
            }
        }
        
    }

    void DrawFrameNumber(Rect rect, PlayableFrameClipData clipData, int frame) {
        Assert.IsNotNull(clipData);
        
        TimelineClip clip = clipData.GetOwner();
        if (null == clip)
            return;
        
        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        if (null == sceneCachePlayableAsset)
            return;

        
        Graphics.DrawTexture(rect, EditorTextures.GetTextBackgroundTexture());
        GUI.Label(rect,frame.ToString());
    }
}

} //end namespace