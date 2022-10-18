using System.Globalization;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor {

[CustomTimelineEditor(typeof(FrameMarker))]
class FrameMarkerEditor : MarkerEditor {

    public override void DrawOverlay(IMarker m, MarkerUIStates uiState, MarkerOverlayRegion region) {
        FrameMarker marker = m as FrameMarker;
        if (null == marker)
            return;

        SISPlayableFrame playableFrame = marker.GetOwner();
        
        //Check invalid PlayableFrame/ClipData. Perhaps because of unsupported Duplicate operation ?
        PlayableFrameClipData clipData = playableFrame?.GetOwner();
        if (clipData == null)
            return;
        
        KeyFrameMode keyFrameMode = (KeyFrameMode) playableFrame.GetProperty(KeyFramePropertyID.Mode);

        const int TEXT_WIDTH = 30;
        Rect      labelRect  = region.markerRegion;
        labelRect.x     += labelRect.width;
        labelRect.width =  TEXT_WIDTH;
        DrawFrameNumber(labelRect, clipData, playableFrame.GetFrameNo());
        
        switch (keyFrameMode) {
            case KeyFrameMode.Smooth: {
                UnityEngine.Graphics.DrawTexture(region.markerRegion, EditorTextures.GetKeyFrameSmoothTexture());
                break;
            }
            case KeyFrameMode.Stop: {
                UnityEngine.Graphics.DrawTexture(region.markerRegion, EditorTextures.GetKeyFrameStopTexture());
                break;
            }
        }
        
    }

    void DrawFrameNumber(Rect rect, PlayableFrameClipData clipData, int frame) {
        Assert.IsNotNull(clipData);
        
        TimelineClip            clip                    = clipData.GetOwner();
        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        if (null == sceneCachePlayableAsset)
            return;

        
        Graphics.DrawTexture(rect, EditorTextures.GetTextBackgroundTexture());
        GUI.Label(rect,frame.ToString());
    }
}

} //end namespace