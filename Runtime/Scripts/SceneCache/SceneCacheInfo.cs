
using UnityEngine;

namespace Unity.MeshSync {

internal class SceneCacheInfo {
    public  int            numFrames;
    private TimeRange      timeRange;
    public  float          sampleRate;
    public  AnimationCurve timeCurve;
}

} //end namespace