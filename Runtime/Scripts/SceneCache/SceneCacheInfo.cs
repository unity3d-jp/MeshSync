
using UnityEngine;

namespace Unity.MeshSync {

internal class SceneCacheInfo : ISceneCacheInfo {

    internal void Reset() {
        numFrames  = 0;
        sampleRate = 0;
        timeRange  = default(TimeRange);
        timeCurve  = null;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    public int            GetNumFrames()  => numFrames;
    public float          GetSampleRate() => sampleRate;
    public TimeRange      GetTimeRange() => timeRange;
    public AnimationCurve GetTimeCurve() => timeCurve;
    
//----------------------------------------------------------------------------------------------------------------------
    
    internal int            numFrames;
    internal float          sampleRate; // value <=0 indicates variable sample rate.
    internal TimeRange      timeRange;
    internal AnimationCurve timeCurve;
}

} //end namespace