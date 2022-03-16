﻿using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync
{

internal class SceneCachePlayableBehaviour : PlayableBehaviour {

    internal void SetSceneCachePlayer(SceneCachePlayer scPlayer) {
        m_sceneCachePlayer = scPlayer;
    }

    internal SceneCachePlayer GetSceneCachePlayer() {
        return m_sceneCachePlayer;        
    }

    internal void SetClipData(SceneCacheClipData clipData) { m_clipData = clipData; } 
    
//----------------------------------------------------------------------------------------------------------------------        
    
    public override void OnBehaviourPlay(Playable playable, FrameData info) {
        if (null == m_sceneCachePlayer)
            return;
        m_sceneCachePlayer.gameObject.SetActive(true);
    }
    

    public override void OnBehaviourPause(Playable playable, FrameData info) {
        if (null == m_sceneCachePlayer)
            return;
        m_sceneCachePlayer.gameObject.SetActive(false);
    }

    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        if (m_sceneCachePlayer.IsNullRef()) {
            return;
        }
        AnimationCurve curve = m_clipData.GetAnimationCurve();
        float normalizedTime = curve.Evaluate((float) playable.GetTime());
              
        m_sceneCachePlayer.SetAutoplay(false);
        m_sceneCachePlayer.SetTimeByNormalizedTime(normalizedTime);
        m_sceneCachePlayer.gameObject.SetActive(true);

    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    private SceneCachePlayer m_sceneCachePlayer = null;
    
    private SceneCacheClipData m_clipData = null;
}

} //end namespace