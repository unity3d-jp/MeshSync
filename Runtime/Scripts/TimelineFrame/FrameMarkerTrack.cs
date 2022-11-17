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
            SceneCacheFrameMarker marker = m as SceneCacheFrameMarker;
            if (null == marker)
                continue;

            if (!marker.Refresh()) {
                m_markersToDelete.Add(marker);                
            }      
        }

        foreach (SceneCacheFrameMarker marker in m_markersToDelete) {
            DeleteMarker(marker);
        }
    }
   
//----------------------------------------------------------------------------------------------------------------------

        
    private readonly List<SceneCacheFrameMarker> m_markersToDelete = new List<SceneCacheFrameMarker>();
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

