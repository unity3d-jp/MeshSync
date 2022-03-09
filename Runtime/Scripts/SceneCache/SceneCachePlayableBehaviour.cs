using Unity.FilmInternalUtilities;
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
    internal void SetSnapToFrame(SnapToFrame snap) { m_snapToFrame = snap; } 
    
//----------------------------------------------------------------------------------------------------------------------        
    

    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        if (m_sceneCachePlayer.IsNullRef()) {
            return;
        }
        AnimationCurve curve = m_clipData.GetAnimationCurve();

        float normalizedTime = 0;
        switch (m_snapToFrame) {
            case SnapToFrame.NONE: {
                normalizedTime = curve.Evaluate((float) playable.GetTime());
                break;                 
            }
            case SnapToFrame.NEAREST: {

                TrackAsset trackAsset = m_clipData.GetOwner().GetParentTrack();
                if (null == trackAsset) {
                    return;
                }
                     
                float fps = (float) trackAsset.timelineAsset.editorSettings.GetFPS();
                
                float timePerFrame = 1.0f / fps;
                int   frame        = Mathf.RoundToInt((float)playable.GetTime() * fps);
                normalizedTime = curve.Evaluate(frame * timePerFrame);
                break;
            }            
        }
              
        m_sceneCachePlayer.SetAutoplay(false);
        m_sceneCachePlayer.SetTimeByNormalizedTime(normalizedTime);

    }

    
//----------------------------------------------------------------------------------------------------------------------
    
    private SceneCachePlayer m_sceneCachePlayer = null;
    
    private SceneCacheClipData m_clipData = null;
    private SnapToFrame        m_snapToFrame;
}

} //end namespace