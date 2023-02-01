using System;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Timeline;

namespace Unity.MeshSync {
[Serializable]
internal class SceneCacheClipData : KeyFrameControllerClipData {
    public SceneCacheClipData() : base() {
    }

    internal SceneCacheClipData(TimelineClip owner, SceneCacheClipData other) : base(owner, other) {
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void OnBeforeSerializeInternalV() {
        base.OnBeforeSerializeInternalV();
        m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION;
    }

    protected override void OnAfterDeserializeInternalV() {
        base.OnAfterDeserializeInternalV();
        m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION;
    }

//----------------------------------------------------------------------------------------------------------------------

    [SerializeField] private AnimationCurve m_animationCurve = AnimationCurve.Constant(0, 0, 0);

    [SerializeField] private LimitedAnimationController m_overrideLimitedAnimationController = new LimitedAnimationController();

#pragma warning disable 414
    [HideInInspector] [SerializeField] private int m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION;
#pragma warning restore 414

//----------------------------------------------------------------------------------------------------------------------

    private const int CUR_SCENE_CACHE_CLIP_DATA_VERSION = (int)SceneCacheClipDataVersion.KeyFrame_0_16_0;

//----------------------------------------------------------------------------------------------------------------------

    internal enum SceneCacheClipDataVersion {
        Initial = 1,
        MovedLimitedAnimationController_0_12_6, //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        MovedAnimationCurve_0_12_6,             //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        KeyFrame_0_16_0                         //Owns KeyFrames in 0.16.0
    }
}
} //end namespace