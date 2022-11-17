using UnityEngine.Timeline;
using System.Collections.Generic;
using Unity.FilmInternalUtilities;

namespace Unity.MeshSync { 
/// <summary>
/// A track which has FrameMarkers and stores extra ClipData for TimelineClips inside Track
/// </summary>
internal abstract class FrameMarkerTrack<T>: BaseExtendedClipTrack<T>
where T: KeyFrameControllerClipData, new()
{

    protected void DeleteInvalidMarkers() {
        foreach(IMarker m in GetMarkers()) {
            KeyFrameMarker marker = m as KeyFrameMarker;
            if (null == marker)
                continue;

            if (!marker.Refresh()) {
                m_markersToDelete.Add(marker);                
            }      
        }

        foreach (KeyFrameMarker marker in m_markersToDelete) {
            DeleteMarker(marker);
        }
    }
   
//----------------------------------------------------------------------------------------------------------------------

        
    private readonly List<KeyFrameMarker> m_markersToDelete = new List<KeyFrameMarker>();
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

