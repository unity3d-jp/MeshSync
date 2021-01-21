using System;
using Unity.FilmInternalUtilities;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class SceneCacheClipData : BaseClipData {

//----------------------------------------------------------------------------------------------------------------------
    
    internal void BindSceneCachePlayer(SceneCachePlayer sceneCachePlayer) {
        if (sceneCachePlayer == m_scPlayer) {
            Debug.Log("SceneCache is already found");
            return;
        }

        m_scPlayer = sceneCachePlayer;
        Debug.Log("SceneCache successfully bound");
    }

    internal void UnbindSceneCachePlayer() {
        m_scPlayer = null;
        Debug.Log("SceneCache successfully UN-bound");
    }

//----------------------------------------------------------------------------------------------------------------------
    internal void SetAnimationCurve(AnimationCurve curve) {
        m_animationCurve = curve;
    }
    
//----------------------------------------------------------------------------------------------------------------------
   
    [SerializeField] private SceneCachePlayer m_scPlayer;
   
    
//----------------------------------------------------------------------------------------------------------------------

    [SerializeField] private AnimationCurve m_animationCurve;


}


} //end namespace


