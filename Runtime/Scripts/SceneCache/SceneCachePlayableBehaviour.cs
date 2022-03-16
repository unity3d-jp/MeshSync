using Unity.FilmInternalUtilities;
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
        float normalizedTime = curve.Evaluate((float) playable.GetTime());
              
        m_sceneCachePlayer.SetAutoplay(false);
        m_sceneCachePlayer.SetTimeByNormalizedTime(normalizedTime);
        m_sceneCachePlayer.gameObject.SetActive(true);

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