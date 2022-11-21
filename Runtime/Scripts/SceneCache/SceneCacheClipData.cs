using System;
using Unity.FilmInternalUtilities;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class SceneCacheClipData : BaseClipData {

//----------------------------------------------------------------------------------------------------------------------
    protected override void OnBeforeSerializeInternalV() {
        m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION;
    }

    protected override void OnAfterDeserializeInternalV() {
        if (m_sceneCacheClipDataVersion == CUR_SCENE_CACHE_CLIP_DATA_VERSION) {
            return;
        }

        if (m_sceneCacheClipDataVersion < (int) SceneCacheClipDataVersion.MovedLimitedAnimationController_0_12_6) {
            m_copyLimitedAnimationControllerToAsset = true;
        }

        if (m_sceneCacheClipDataVersion < (int) SceneCacheClipDataVersion.MovedAnimationCurve_0_12_6) {
            m_copyAnimationCurveToAsset = true;
        }
        
        m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    internal override void DestroyV() { }

//----------------------------------------------------------------------------------------------------------------------
   
    [SerializeField] private AnimationCurve m_animationCurve = AnimationCurve.Constant(0,0,0);
    [SerializeField] private bool           m_initialized    = false;

    [SerializeField] private LimitedAnimationController m_overrideLimitedAnimationController = new LimitedAnimationController();

#pragma warning disable 414    
    [HideInInspector][SerializeField] private int m_sceneCacheClipDataVersion = CUR_SCENE_CACHE_CLIP_DATA_VERSION; 
#pragma warning restore 414    
    
//----------------------------------------------------------------------------------------------------------------------
    
    private const int CUR_SCENE_CACHE_CLIP_DATA_VERSION = (int) SceneCacheClipDataVersion.MovedAnimationCurve_0_12_6;

    private bool m_copyLimitedAnimationControllerToAsset = false;
    private bool m_copyAnimationCurveToAsset = false;

//----------------------------------------------------------------------------------------------------------------------

    internal enum SceneCacheClipDataVersion {
        Initial = 1, 
        MovedLimitedAnimationController_0_12_6, //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        MovedAnimationCurve_0_12_6, //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        
    } 
    
}


} //end namespace


