using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.StreamingImageSequence {

// A PlayableBehaviour that is attached to a Track via CreateTrackMixer() 
internal abstract class BasePlayableMixer<T> : PlayableBehaviour where T: PlayableAsset {

    #region IPlayableBehaviour interfaces
    
    public override void OnPlayableDestroy(Playable playable) {
        base.OnPlayableDestroy(playable);
        Destroy();
    }
    
    public override void PrepareFrame(Playable playable, FrameData info) {
        base.PrepareFrame(playable, info);
        ShowObjectV(false);
    }
    

//----------------------------------------------------------------------------------------------------------------------
    // Called each frame while the state is set to Play
    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        int inputCount = playable.GetInputCount<Playable>();
        if (inputCount == 0 ) {
            return; // it doesn't work as mixer.
        }

        
        if (m_boundGameObject== null ) {
            return;
        }

        GetActiveTimelineClipInto(m_clips, m_playableDirector.time, out TimelineClip clip, out T activePlayableAsset);        
        if (null == clip)
            return;
        
        ProcessActiveClipV(activePlayableAsset, m_playableDirector.time, clip);

        ShowObjectV(true);
        
    }

    #endregion IPlayableBehaviour interfaces
    

//----------------------------------------------------------------------------------------------------------------------
    
    internal static void GetActiveTimelineClipInto( IList<TimelineClip> sortedClips, double directorTime, 
        out TimelineClip outClip, out T outAsset) {

        TimelineClip prevClipWithPostExtrapolation = null;
        TimelineClip nextClipWithPreExtrapolation  = null;
        bool         nextClipChecked               = false; 
               
        foreach (TimelineClip clip in sortedClips) {


            if (directorTime < clip.start) {
                //must check only once since we loop from the start
                if (!nextClipChecked) { 
                    //store next direct clip which has PreExtrapolation
                    nextClipWithPreExtrapolation = clip.hasPreExtrapolation ? clip : null;
                    nextClipChecked              = true;
                }

                continue;
            }

            if (clip.end < directorTime) {
                //store prev direct clip which has PostExtrapolation
                prevClipWithPostExtrapolation = clip.hasPostExtrapolation ? clip : null;
                continue;                
            }

            outClip  = clip;
            outAsset = clip.asset as T;
            return;
        }
        
        
        //check for post-extrapolation
        if (null != prevClipWithPostExtrapolation) {
            outClip  = prevClipWithPostExtrapolation;
            outAsset = prevClipWithPostExtrapolation.asset as T;
            return;
        }

        //check pre-extrapolation for the first clip
        if (null!=nextClipWithPreExtrapolation) {
            outClip  = nextClipWithPreExtrapolation;
            outAsset = nextClipWithPreExtrapolation.asset as T;
            return;
        }        
        outClip  = null;
        outAsset = null;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    internal void Init(GameObject go, PlayableDirector director, IEnumerable<TimelineClip> clips) {
        m_boundGameObject = go;
        m_playableDirector = director;
        
        m_clips = new List<TimelineClip>(clips);
        m_clipAssets = new Dictionary<TimelineClip, T>();
        foreach (TimelineClip clip in m_clips) {
            T clipAsset = clip.asset as T;
            Assert.IsNotNull(clipAsset);
            m_clipAssets.Add(clip, clipAsset);
        }
        

        InitInternalV(go);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    internal void Destroy() {
        m_clips.Clear();
        m_clipAssets.Clear();
        
    }

//----------------------------------------------------------------------------------------------------------------------
    internal double GetDirectorTime() { return null!= m_playableDirector ?  m_playableDirector.time : 0; } 
    
//----------------------------------------------------------------------------------------------------------------------

    protected abstract void InitInternalV(GameObject go);
    protected abstract void ProcessActiveClipV(T asset, double directorTime, TimelineClip activeClip);
    
    protected abstract void ShowObjectV(bool show);
    

//----------------------------------------------------------------------------------------------------------------------
    protected PlayableDirector GetPlayableDirector() { return m_playableDirector; }
    protected IEnumerable<TimelineClip> GetClips() { return m_clips; }
    internal IEnumerable<KeyValuePair<TimelineClip,T>> GetClipAssets() { return m_clipAssets; }

//----------------------------------------------------------------------------------------------------------------------

    private GameObject m_boundGameObject;
    private PlayableDirector m_playableDirector;
    private List<TimelineClip> m_clips;
    private Dictionary<TimelineClip, T> m_clipAssets;

}

} //end namespace