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
        if (null == m_sceneCachePlayer) {
            return;
        }
        AnimationCurve curve = m_clipData.GetAnimationCurve();
        
        double t = CalculateTimeForLimitedAnimation((float) playable.GetTime());        
        float normalizedTime = curve.Evaluate((float)t);
              
        m_sceneCachePlayer.SetAutoplay(false);
        m_sceneCachePlayer.SetTimeByNormalizedTime(normalizedTime);        
    }

    private double CalculateTimeForLimitedAnimation(double time) {
        LimitedAnimationController origLimitedAnimationController = m_sceneCachePlayer.GetLimitedAnimationController();
        if (origLimitedAnimationController.IsEnabled()) //do nothing if LA is set on the target SceneCache
            return time;
        
        LimitedAnimationController clipLimitedAnimationController = m_clipData.GetOverrideLimitedAnimationController();
        if (!clipLimitedAnimationController.IsEnabled())
            return time;

        ISceneCacheInfo scInfo = m_sceneCachePlayer.ExtractSceneCacheInfo(forceOpen: true);
        if (null == scInfo)
            return time;
            
        int frame = m_sceneCachePlayer.CalculateFrame((float)time,clipLimitedAnimationController);
        return frame / scInfo.GetSampleRate();
    }

//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer m_sceneCachePlayer  = null;
    
    private SceneCacheClipData m_clipData = null;
    
    
}

} //end namespace