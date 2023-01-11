using UnityEngine;

namespace Unity.MeshSync {
internal class SceneCacheInfo : ISceneCacheInfo {
    internal void Reset() {
        numFrames  = 0;
        sampleRate = 0;
        timeRange  = default;
        timeCurve  = null;
    }

//----------------------------------------------------------------------------------------------------------------------

    public int GetNumFrames() {
        return numFrames;
    }

    public float GetSampleRate() {
        return sampleRate;
    }

    public TimeRange GetTimeRange() {
        return timeRange;
    }

    public AnimationCurve GetTimeCurve() {
        return timeCurve;
    }

    //----------------------------------------------------------------------------------------------------------------------

    internal int            numFrames;
    internal float          sampleRate; // value <=0 indicates variable sample rate.
    internal TimeRange      timeRange;
    internal AnimationCurve timeCurve;
}
} //end namespace