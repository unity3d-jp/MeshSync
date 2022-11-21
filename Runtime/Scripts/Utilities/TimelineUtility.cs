using System.Collections.Generic;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

internal static class TimelineUtility  {

    static void DeleteInvalidMarkers(TrackAsset track) {
        List<KeyFrameMarker> markersToDelete = new List<KeyFrameMarker>();
        foreach(IMarker m in track.GetMarkers()) {
            KeyFrameMarker marker = m as KeyFrameMarker;
            if (null == marker)
                continue;

            if (!marker.Refresh()) {
                markersToDelete.Add(marker);                
            }      
        }

        foreach (KeyFrameMarker marker in markersToDelete) {
            track.DeleteMarker(marker);
        }
    }
    
}


} //end namespace
