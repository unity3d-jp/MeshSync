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

        float          localTime = (float) playable.GetTime();
        SceneCacheInfo scInfo    = m_sceneCachePlayer.ExtractSceneCacheInfo(forceOpen: true);
        if (null == scInfo)
            return;

        float                  sceneCacheTime = 0;
        SceneCachePlaybackMode playbackMode   = m_sceneCachePlayer.GetPlaybackMode();
        
        //LimitedAnimationController (obsolete!)
        if (limitedAnimationController.IsEnabled()) {
            int sceneCacheFrame = m_sceneCachePlayer.CalculateFrame((float)localTime,limitedAnimationController);
            sceneCacheTime  = sceneCacheFrame / scInfo.GetSampleRate();
            m_sceneCachePlayer.SetTime(sceneCacheTime);
            return;
        }
        
        AnimationCurve curve          = m_sceneCachePlayableAsset.GetAnimationCurve();
        float          normalizedTime = curve.Evaluate(localTime);
        
        float estimatedTime = (normalizedTime * scInfo.GetTimeRange().end);
        sceneCacheTime = SceneCachePlayer.CalculatePlaybackTime(estimatedTime,playbackMode, scInfo);
        m_sceneCachePlayer.SetTime(sceneCacheTime);
        
        //Debug.Log($"MotionNormalizedTime: {normalizedTime}. Frame: {sceneCacheFrame}. LocalTime: {localTime}.");
    }

//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer        m_sceneCachePlayer        = null;    
    private SceneCachePlayableAsset m_sceneCachePlayableAsset = null;
    
}

} //end namespace