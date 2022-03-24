using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {

// A behaviour that is attached to a playable
internal class SceneCachePlayableMixer : PlayableBehaviour {
    
    internal void Init(PlayableDirector director, SceneCacheTrack track, IEnumerable<TimelineClip> clips) {
        m_playableDirector = director;
        m_sceneCacheTrack  = track;
        
        m_clips      = new List<TimelineClip>(clips);
        m_clipAssets = new Dictionary<TimelineClip, SceneCachePlayableAsset>();
        foreach (TimelineClip clip in m_clips) {
            SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
            Assert.IsNotNull(sceneCachePlayableAsset);
            m_clipAssets.Add(clip, sceneCachePlayableAsset);
        }
    }
    
    internal void Destroy() {
        m_clips.Clear();
        m_clipAssets.Clear();
    }    
//----------------------------------------------------------------------------------------------------------------------
    
    #region PlayableBehaviour interfaces
    
    public override void OnPlayableDestroy(Playable playable) {
        base.OnPlayableDestroy(playable);
        Destroy();
    }    
    
    public override void PrepareFrame(Playable playable, FrameData info) {
        base.PrepareFrame(playable, info);

        m_inactiveSceneCacheObjects.Clear();

        int curFrame = Time.frameCount;
        if (m_lastPrepareGlobalTime != curFrame) {
            m_activatedSceneCacheObjectsInFrame.Clear();
            m_lastPrepareGlobalTime = curFrame;
        }

        //Register all SceneCache objects as inactive
        foreach (var clipData in m_clipAssets.Values) {
            SceneCachePlayer scPlayer = clipData.GetSceneCachePlayer();
            if (null == scPlayer)
                continue;

            m_inactiveSceneCacheObjects.Add(scPlayer.gameObject);
        }
    }

    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        
        GetActiveTimelineClipInto(m_clips, m_playableDirector.time, out TimelineClip clip, out SceneCachePlayableAsset scPlayableAsset);
        if (null == clip) {
            UpdateObjectActiveStates();
            return;
        }

        SceneCacheClipData clipData = scPlayableAsset.GetBoundClipData();
        Assert.IsNotNull(clipData);

        SceneCachePlayer scPlayer = scPlayableAsset.GetSceneCachePlayer();
        if (null == scPlayer) {
            UpdateObjectActiveStates();
            return;
        }

        UpdateObjectActiveStates(activeObject: scPlayer.gameObject);
        LimitedAnimationController limitedAnimationController = clipData.GetOverrideLimitedAnimationController(); 

        
        double localTime = clip.ToLocalTime(playable.GetTime());
        double t         = CalculateTimeForLimitedAnimation(scPlayer,limitedAnimationController, localTime);
        
        AnimationCurve curve          = scPlayableAsset.GetAnimationCurve();
        float          normalizedTime = curve.Evaluate((float)t);
              
        scPlayer.SetAutoplay(false);
        scPlayer.SetTimeByNormalizedTime(normalizedTime);
        
    }

    #endregion PlayableBehaviour interfaces


    void UpdateObjectActiveStates(GameObject activeObject = null) {

        if (!m_sceneCacheTrack.IsAutoActivateObject())
            return;

        //Previously activated objects should stay active
        foreach (GameObject go in m_activatedSceneCacheObjectsInFrame)
            m_inactiveSceneCacheObjects.Remove(go);
        
        if (null != activeObject) {
            activeObject.SetActive(true);
            m_activatedSceneCacheObjectsInFrame.Add(activeObject);
            m_inactiveSceneCacheObjects.Remove(activeObject);
        }
        foreach (GameObject go in m_inactiveSceneCacheObjects) {
            go.SetActive(false);
        }
        
    }

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

            if (clip.end <= directorTime) {
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
    
    private PlayableDirector   m_playableDirector;
    private SceneCacheTrack    m_sceneCacheTrack;
    private List<TimelineClip> m_clips;
    
    private Dictionary<TimelineClip, SceneCachePlayableAsset> m_clipAssets;

    private readonly HashSet<GameObject> m_inactiveSceneCacheObjects = new HashSet<GameObject>();
    
    private static readonly HashSet<GameObject> m_activatedSceneCacheObjectsInFrame = new HashSet<GameObject>();
    private static          int                 m_lastPrepareGlobalTime             = 0;

}

} //end namespace