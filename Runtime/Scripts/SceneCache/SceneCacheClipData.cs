using System;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Timeline;

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
   
    [SerializeField] private SceneCachePlayer m_scPlayer;
    
}


} //end namespace


