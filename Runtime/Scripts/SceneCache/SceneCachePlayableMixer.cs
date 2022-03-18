using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

// A behaviour that is attached to a playable
internal class SceneCachePlayableMixer : PlayableBehaviour {
    
    internal void Init(PlayableDirector director, IEnumerable<TimelineClip> clips) {
        m_playableDirector = director;
        
        m_clips      = new List<TimelineClip>(clips);
        m_clipDataDictionary = new Dictionary<TimelineClip, SceneCacheClipData>();
        foreach (TimelineClip clip in m_clips) {
            SceneCacheClipData clipData = clip.GetClipData<SceneCacheClipData>();
            Assert.IsNotNull(clipData);
            m_clipDataDictionary.Add(clip, clipData);
        }
    }
    
    internal void Destroy() {
        m_clips.Clear();
        m_clipDataDictionary.Clear();        
    }    
//----------------------------------------------------------------------------------------------------------------------
    
    #region PlayableBehaviour interfaces
    
    public override void OnPlayableDestroy(Playable playable) {
        base.OnPlayableDestroy(playable);
        Destroy();
    }    
    
    public override void PrepareFrame(Playable playable, FrameData info) {
        base.PrepareFrame(playable, info);

        //Hide all SceneCache objects
        foreach (var clipData in m_clipDataDictionary.Values) {
            SceneCachePlayer scPlayer = clipData.GetSceneCachePlayer();
            if (null == scPlayer)
                continue;
            scPlayer.gameObject.SetActive(false);

        }
    }

    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        
        int inputCount = playable.GetInputCount<Playable>();
        if (inputCount == 0 ) {
            return; // it doesn't work as mixer.
        }

        GetActiveTimelineClipInto(m_clips, m_playableDirector.time, out TimelineClip clip, out SceneCachePlayableAsset activePlayableAsset);        
        if (null == clip)
            return;

        SceneCacheClipData clipData = activePlayableAsset.GetBoundClipData();
        Assert.IsNotNull(clipData);

        SceneCachePlayer scPlayer = clipData.GetSceneCachePlayer();
        if (null == scPlayer)
            return;
        
        scPlayer.gameObject.SetActive(true);
        
        AnimationCurve curve = clipData.GetAnimationCurve();
        
        double t              = CalculateTimeForLimitedAnimation(clipData,(float) playable.GetTime());        
        float  normalizedTime = curve.Evaluate((float)t);
              
        scPlayer.SetAutoplay(false);
        scPlayer.SetTimeByNormalizedTime(normalizedTime);        
        
    }

    #endregion PlayableBehaviour interfaces


//----------------------------------------------------------------------------------------------------------------------
    
    //[TODO-sin: 2022-3-18] Move to FilmInternalUtilities
    static void GetActiveTimelineClipInto<T>( IList<TimelineClip> sortedClips, double directorTime, 
        out TimelineClip outClip, out T outAsset) where T: PlayableAsset 
    {

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
    
    private static double CalculateTimeForLimitedAnimation(SceneCacheClipData clipData, double time) {

        SceneCachePlayer scPlayer = clipData.GetSceneCachePlayer();
        if (null == scPlayer)
            return time;
        
        
        LimitedAnimationController origLimitedAnimationController = scPlayer.GetLimitedAnimationController();
        if (origLimitedAnimationController.IsEnabled()) //do nothing if LA is set on the target SceneCache
            return time;
        
        LimitedAnimationController clipLimitedAnimationController = clipData.GetOverrideLimitedAnimationController();
        if (!clipLimitedAnimationController.IsEnabled())
            return time;

        ISceneCacheInfo scInfo = scPlayer.ExtractSceneCacheInfo(forceOpen: true);
        if (null == scInfo)
            return time;
            
        int frame = scPlayer.CalculateFrame((float)time,clipLimitedAnimationController);
        return frame / scInfo.GetSampleRate();
    }
    
//----------------------------------------------------------------------------------------------------------------------    

    private PlayableDirector   m_playableDirector;
    private List<TimelineClip> m_clips;
    
    private Dictionary<TimelineClip, SceneCacheClipData> m_clipDataDictionary;
    
    
}

} //end namespace