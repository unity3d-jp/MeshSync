﻿using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Playables;

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
    
    public override void OnPlayableDestroy(Playable playable) {
        //Check if OnBehaviourPause() was disabling the gameobject when the playable is deleted
        if (!m_isGameObjectActive && m_lastActiveChangedTime == Time.frameCount) {
            ActivateGameObject(true);
        }
    }
    
    public override void OnBehaviourPlay(Playable playable, FrameData info) {
        ActivateGameObject(true);
    }
    

    public override void OnBehaviourPause(Playable playable, FrameData info) {        
        ActivateGameObject(false);
    }
    
    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        if (null == m_sceneCachePlayer) {
            return;
        }
        AnimationCurve curve = m_clipData.GetAnimationCurve();
        
        double t = CalculateTimeForLimitedAnimation((float) playable.GetTime());        
        float normalizedTime = curve.Evaluate((float)t);
              
        m_sceneCachePlayer.SetAutoplay(false);
        m_sceneCachePlayer.SetTimeByNormalizedTime(normalizedTime);
        
        //Might have been deactivated if there is another clip with the same SceneCache in the same track
        ActivateGameObject(true); 
    }

    private double CalculateTimeForLimitedAnimation(double time) {
        LimitedAnimationController origLimitedAnimationController = m_sceneCachePlayer.GetLimitedAnimationController();
        if (origLimitedAnimationController.IsEnabled()) //do nothing if LA is set on the target SceneCache
            return time;
        
        LimitedAnimationController clipLimitedAnimationController = m_clipData.GetOverrideLimitedAnimationController();
        if (!clipLimitedAnimationController.IsEnabled())
            return time;

        SceneCacheInfo scInfo = m_sceneCachePlayer.ExtractSceneCacheInfo(forceOpen: true);
        if (null == scInfo)
            return time;
            
        int frame = m_sceneCachePlayer.CalculateFrame((float)time,clipLimitedAnimationController);
        return frame / scInfo.sampleRate;
    }

//----------------------------------------------------------------------------------------------------------------------

    private void ActivateGameObject(bool active) {
        if (null == m_sceneCachePlayer)
            return;

        GameObject go = m_sceneCachePlayer.gameObject;
        if (go.activeSelf == active)
            return;
        
        go.SetActive(active);
        
        m_isGameObjectActive    = go.activeSelf;
        m_lastActiveChangedTime = Time.frameCount;
    }
//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer m_sceneCachePlayer  = null;
    
    private SceneCacheClipData m_clipData = null;
    
    private int m_lastActiveChangedTime = 0;
    private bool  m_isGameObjectActive = false;
    
}

} //end namespace