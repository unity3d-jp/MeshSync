using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Timeline;
using UnityEngine.Playables;

namespace Unity.StreamingImageSequence {

// A behaviour that is attached to a playable
internal class SceneCachePlayableMixer : BasePlayableMixer<SceneCachePlayableAsset> {

#if false //PlayableBehaviour's functions that can be overridden

    // Called when the owning graph starts playing
    public override void OnGraphStart(Playable playable) {
    }


    // Called when the owning graph stops playing
    public override void OnGraphStop(Playable playable) {
    }

    // Called when the state of the playable is set to Play
    public override void OnBehaviourPlay(Playable playable, FrameData info) {

    }

    // Called when the state of the playable is set to Paused
    public override void OnBehaviourPause(Playable playable, FrameData info) {

    }

#endif


//----------------------------------------------------------------------------------------------------------------------

    protected override void InitInternalV() {       

        //m_image = gameObject.GetComponent<Image>();
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void ProcessActiveClipV(SceneCachePlayableAsset asset, 
        double directorTime, TimelineClip activeClip) 
    {

        Color color = asset.GetColor();
        float maxFade = color.a;

        float fade = (float)( ((directorTime - activeClip.start) / activeClip.duration ) * maxFade);
        if ( asset.GetFadeType() == FadeType.FADE_OUT) {
            fade = maxFade - fade;
        }

        color.a = fade;
    }

//----------------------------------------------------------------------------------------------------------------------    
    protected override void ShowObjectV(bool show) {
        // if (null!=m_image) {
        //     m_image.enabled = show;
        // } 
    }
    
//----------------------------------------------------------------------------------------------------------------------


}

} //end namespace