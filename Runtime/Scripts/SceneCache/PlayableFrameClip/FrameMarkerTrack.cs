using UnityEngine.Timeline;
using System.Collections.Generic;
using Unity.FilmInternalUtilities;

namespace Unity.MeshSync { 
/// <summary>
/// A track which has FrameMarkers and stores extra ClipData for TimelineClips inside Track
/// </summary>
internal abstract class FrameMarkerTrack<T>: BaseExtendedClipTrack<T>
where T: PlayableFrameClipData, new()
{

    protected void DeleteInvalidMarkers() {
        foreach(IMarker m in GetMarkers()) {
            FrameMarker marker = m as FrameMarker;
            if (null == marker)
                continue;

            if (!marker.Refresh()) {
                m_markersToDelete.Add(marker);                
            }      
        }

        foreach (FrameMarker marker in m_markersToDelete) {
            DeleteMarker(marker);
        }
    }
   
//----------------------------------------------------------------------------------------------------------------------

        
    private readonly List<FrameMarker> m_markersToDelete = new List<FrameMarker>();
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

