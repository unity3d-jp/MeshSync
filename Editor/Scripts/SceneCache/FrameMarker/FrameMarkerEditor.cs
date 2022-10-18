using System.Globalization;
using UnityEditor.Timeline;
using UnityEngine;
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


        Graphics.DrawTexture(labelRect, EditorTextures.GetTextBackgroundTexture());
        GUI.Label(labelRect,playableFrame.GetNormalizedAnimationTime().ToString(CultureInfo.InvariantCulture));
        
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
}

} //end namespace