using UnityEngine.Timeline;

namespace Unity.FilmInternalUtilities
{    
internal abstract class BaseTrack : TrackAsset {
    
    internal virtual int GetCapsV() { return 0; }    
}

} //end namespace
