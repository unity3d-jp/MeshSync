using System.Collections.Generic;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

//[TODO-sin: 2022-11-21] Move to FIU
internal static class TimelineUtility {

    internal static void DeleteInvalidMarkers<MarkerType>(TrackAsset track) where MarkerType : Marker, ICanRefresh {
        List<Marker> markersToDelete = new List<Marker>();
        foreach(IMarker m in track.GetMarkers()) {
            MarkerType marker = m as MarkerType;
            if (null == marker)
                continue;

            if (!marker.Refresh()) {
                markersToDelete.Add(marker);
            }
        }

        foreach (Marker marker in markersToDelete) {
            track.DeleteMarker(marker);
        }
    }
    
}


} //end namespace
