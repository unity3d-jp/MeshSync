using UnityEngine.Timeline;

namespace Unity.StreamingImageSequence
{
    
internal abstract class BaseSISTrack : TrackAsset {
    internal virtual SISTrackCaps GetCapsV() { return SISTrackCaps.NONE; }    
}

} //end namespace
