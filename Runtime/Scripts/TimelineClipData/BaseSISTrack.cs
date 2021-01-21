using UnityEngine.Timeline;

namespace Unity.FilmInternalUtilities
{
    
internal abstract class BaseSISTrack : TrackAsset {
    internal virtual SISTrackCaps GetCapsV() { return SISTrackCaps.NONE; }    
}

} //end namespace
