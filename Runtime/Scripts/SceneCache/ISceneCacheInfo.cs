using UnityEngine;

namespace Unity.MeshSync {
internal interface ISceneCacheInfo {
    int            GetNumFrames();
    float          GetSampleRate();
    TimeRange      GetTimeRange();
    AnimationCurve GetTimeCurve();
}
} //end namespace