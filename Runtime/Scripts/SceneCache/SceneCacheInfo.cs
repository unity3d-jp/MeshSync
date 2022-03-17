
using UnityEngine;

namespace Unity.MeshSync {

internal class SceneCacheInfo {
    internal int            numFrames;
    internal float          sampleRate; // value <=0 indicates variable sample rate.
    internal TimeRange      timeRange;
    internal AnimationCurve timeCurve;
}

} //end namespace