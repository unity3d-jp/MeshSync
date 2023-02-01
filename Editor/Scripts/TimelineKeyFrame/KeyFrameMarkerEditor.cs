using System.Globalization;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor {
[CustomTimelineEditor(typeof(KeyFrameMarker))]
internal class KeyFrameMarkerEditor : MarkerEditor {
    public override void DrawOverlay(IMarker m, MarkerUIStates uiState, MarkerOverlayRegion region) {
        KeyFrameMarker marker = m as KeyFrameMarker;
        if (null == marker)
            return;

        PlayableKeyFrame playableKeyFrame = marker.GetOwner();

        //Check invalid PlayableFrame/ClipData. Perhaps because of unsupported Duplicate operation ?
        KeyFrameControllerClipData clipData = playableKeyFrame?.GetOwner();
        if (clipData == null)
            return;

        KeyFrameMode keyFrameMode = playableKeyFrame.GetKeyFrameMode();

        const int TEXT_WIDTH = 30;
        Rect      labelRect  = region.markerRegion;
        labelRect.x     += labelRect.width;
        labelRect.width =  TEXT_WIDTH;
        DrawFrameNumber(labelRect, clipData, playableKeyFrame.GetPlayFrame());

        switch (keyFrameMode) {
            case KeyFrameMode.Continuous: {
                Graphics.DrawTexture(region.markerRegion, EditorTextures.GetKeyFrameSmoothTexture());
                break;
            }
            case KeyFrameMode.Hold: {
                Graphics.DrawTexture(region.markerRegion, EditorTextures.GetKeyFrameStopTexture());
                break;
            }
        }
    }

    private void DrawFrameNumber(Rect rect, KeyFrameControllerClipData clipData, int frame) {
        Assert.IsNotNull(clipData);

        TimelineClip clip = clipData.GetOwner();
        if (null == clip)
            return;

        SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
        if (null == sceneCachePlayableAsset)
            return;


        Graphics.DrawTexture(rect, EditorTextures.GetTextBackgroundTexture());
        GUI.Label(rect, frame.ToString());
    }
}
} //end namespace