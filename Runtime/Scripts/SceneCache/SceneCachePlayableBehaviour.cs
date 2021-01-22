using Unity.AnimeToolbox;
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
    

    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        if (m_sceneCachePlayer.IsNullRef()) {
            return;
        }
        
        
        
        float normalizedTime = (float)( playable.GetTime() / playable.GetDuration());        
        m_sceneCachePlayer.RequestNormalizedTime(normalizedTime);
        
        Debug.Log(m_clipData.GetAnimationCurve().keys.Length);

    }

    
//----------------------------------------------------------------------------------------------------------------------        
    private SceneCachePlayer m_sceneCachePlayer = null;
    
    private SceneCacheClipData m_clipData = null;
    
}

} //end namespace