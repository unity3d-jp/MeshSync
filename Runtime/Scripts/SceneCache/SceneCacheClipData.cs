﻿using System;
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

//----------------------------------------------------------------------------------------------------------------------

    internal enum SceneCacheClipDataVersion {
        Initial = 1, 
        MovedLimitedAnimationController_0_12_6, //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        MovedAnimationCurve_0_12_6, //Moved LimitedAnimationController to SceneCachePlayableAsset in 0.12.6
        
    } 
    
}


} //end namespace


