using UnityEngine.Timeline;

namespace Unity.FilmInternalUtilities
{    
internal abstract class BaseFilmTrack : TrackAsset {
    
    internal virtual int GetCapsV() { return 0; }    
}

} //end namespace
