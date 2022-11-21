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
    
    /// <inheritdoc/>
    public override void OnGraphStart(Playable playable) {
        m_sceneCachePlayableAsset.OnGraphStart(playable);
    }
    
    public override void OnPlayableDestroy(Playable playable) { }
    
    public override void OnBehaviourPlay(Playable playable, FrameData info) { }
    

    public override void OnBehaviourPause(Playable playable, FrameData info) { }
    
    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        if (null == m_sceneCachePlayer) {
            return;
        }
        
        LimitedAnimationController limitedAnimationController = m_sceneCachePlayableAsset.GetOverrideLimitedAnimationController();

        float localTime = (float) playable.GetTime(); 
        
        //LimitedAnimationController (obsolete!)
        if (limitedAnimationController.IsEnabled()) {
            ISceneCacheInfo scInfo = m_sceneCachePlayer.ExtractSceneCacheInfo(forceOpen: true);
            if (null != scInfo) {
                int frame = m_sceneCachePlayer.CalculateFrame((float)localTime,limitedAnimationController);
                localTime = frame / scInfo.GetSampleRate(); 
            }
            m_sceneCachePlayer.SetTime(localTime);
            return;
        }
        
        AnimationCurve curve          = m_sceneCachePlayableAsset.GetAnimationCurve();
        float          normalizedTime = curve.Evaluate((float)localTime);
        m_sceneCachePlayer.SetTimeByNormalizedTime(normalizedTime);
        
        //Debug.Log($"MotionNormalizedTime: {normalizedTime}. LocalTime: {t}");
    }

//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer        m_sceneCachePlayer        = null;    
    private SceneCachePlayableAsset m_sceneCachePlayableAsset = null;
    
}

} //end namespace