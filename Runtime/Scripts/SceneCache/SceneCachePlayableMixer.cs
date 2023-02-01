using System.Collections.Generic;
using Unity.FilmInternalUtilities;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Unity.MeshSync {
internal class SceneCachePlayableMixer : PlayableBehaviour {
    internal void Init(PlayableDirector director, SceneCacheTrack track) {
        m_playableDirector = director;
        m_sceneCacheTrack  = track;

        m_clips      = new List<TimelineClip>(track.GetClips());
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
        foreach (SceneCachePlayableAsset sceneCachePlayableAsset in m_clipAssets.Values) {
            SceneCachePlayer scPlayer = sceneCachePlayableAsset.GetSceneCachePlayer();
            if (null == scPlayer)
                continue;

            m_inactiveSceneCacheObjects.Add(scPlayer.gameObject);
        }
    }

    public override void ProcessFrame(Playable playable, FrameData info, object playerData) {
        FilmInternalUtilities.TimelineUtility.GetActiveTimelineClipInto(m_clips, m_playableDirector.time, out TimelineClip clip,
            out SceneCachePlayableAsset activePlayableAsset);
        if (null == clip) {
            UpdateObjectActiveStates();
            return;
        }


        SceneCacheClipData clipData = activePlayableAsset.GetBoundClipData();
        Assert.IsNotNull(clipData);

        SceneCachePlayer scPlayer = activePlayableAsset.GetSceneCachePlayer();
        if (null == scPlayer) {
            UpdateObjectActiveStates();
            return;
        }

        UpdateObjectActiveStates(scPlayer.gameObject);
    }

    #endregion PlayableBehaviour interfaces


    private void UpdateObjectActiveStates(GameObject activeObject = null) {
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

        foreach (GameObject go in m_inactiveSceneCacheObjects) go.SetActive(false);
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