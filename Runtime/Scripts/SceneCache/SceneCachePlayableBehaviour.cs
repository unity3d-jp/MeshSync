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
    
//----------------------------------------------------------------------------------------------------------------------        
    public override void PrepareFrame(Playable playable, FrameData info) {
        base.PrepareFrame(playable, info);
    }
    

    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        float normalizedTime = (float)( playable.GetTime() / playable.GetDuration());        
        m_sceneCachePlayer.SetNormalizedTime(normalizedTime);        
    }

    
//----------------------------------------------------------------------------------------------------------------------        
    private SceneCachePlayer m_sceneCachePlayer;
}

} //end namespace