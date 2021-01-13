using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Timeline;
using UnityEngine.Playables;

namespace Unity.MeshSync {

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

    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void ProcessActiveClipV(SceneCachePlayableAsset asset, 
        double directorTime, TimelineClip activeClip) 
    {

    }

//----------------------------------------------------------------------------------------------------------------------    
    protected override void ShowObjectV(bool show) {
    }
    
//----------------------------------------------------------------------------------------------------------------------


}

} //end namespace