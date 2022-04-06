using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Playables;

namespace Unity.MeshSync
{

internal class SceneCachePlayableBehaviour : PlayableBehaviour {

    internal void SetSceneCachePlayer(SceneCachePlayer scPlayer) {
        m_sceneCachePlayer = scPlayer;
    }

    internal void SetPlayableAsset(SceneCachePlayableAsset playableAsset) {
        m_sceneCachePlayableAsset = playableAsset;
    } 
    
//----------------------------------------------------------------------------------------------------------------------        
    
    public override void OnPlayableDestroy(Playable playable) { }
    
    public override void OnBehaviourPlay(Playable playable, FrameData info) { }
    

    public override void OnBehaviourPause(Playable playable, FrameData info) { }
    
    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        if (null == m_sceneCachePlayer) {
            return;
        }
        
        LimitedAnimationController limitedAnimationController = m_sceneCachePlayableAsset.GetOverrideLimitedAnimationController(); 
        
        double localTime = playable.GetTime();
        double t         = CalculateTimeForLimitedAnimation(m_sceneCachePlayer,limitedAnimationController, localTime);
        
        AnimationCurve curve          = m_sceneCachePlayableAsset.GetAnimationCurve();
        float          normalizedTime = curve.Evaluate((float)t);
              
        m_sceneCachePlayer.SetAutoplay(false);
        m_sceneCachePlayer.SetTimeByNormalizedTime(normalizedTime);

    }
    
//----------------------------------------------------------------------------------------------------------------------        

    private static double CalculateTimeForLimitedAnimation(SceneCachePlayer scPlayer, 
        LimitedAnimationController overrideLimitedAnimationController, double time)  
    {
        LimitedAnimationController origLimitedAnimationController = scPlayer.GetLimitedAnimationController();
        if (origLimitedAnimationController.IsEnabled()) //do nothing if LA is set on the target SceneCache
            return time;
        
        if (!overrideLimitedAnimationController.IsEnabled())
            return time;

        ISceneCacheInfo scInfo = scPlayer.ExtractSceneCacheInfo(forceOpen: true);
        if (null == scInfo)
            return time;
            
        int frame = scPlayer.CalculateFrame((float)time,overrideLimitedAnimationController);
        return frame / scInfo.GetSampleRate();
    }

//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer        m_sceneCachePlayer        = null;    
    private SceneCachePlayableAsset m_sceneCachePlayableAsset = null;
    
}

} //end namespace