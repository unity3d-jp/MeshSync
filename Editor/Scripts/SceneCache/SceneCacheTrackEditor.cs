using System.Collections.Generic;
using UnityEditor.Timeline;
using UnityEngine.Assertions;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor {
[CustomTimelineEditor(typeof(SceneCacheTrack))]
internal class SceneCacheTrackEditor : TrackEditor {
    public override void OnTrackChanged(TrackAsset track) {
        if (!WasMuted(track))
            return;

        //[Note-sin:2022-9-27] if the same SceneCache exists in another track in the same time, then this will deactivate and activate the object again.
        //Ideally, we should detect if it's possible to avoid this, but this doesn't seem possible at the moment.
        //Event order: Editor.OnTrackChanged() -> Mixer.PrepareFrame() -> Mixer.ProcessFrame()
        SceneCacheTrack sceneCacheTrack = track as SceneCacheTrack;
        Assert.IsNotNull(sceneCacheTrack);
        if (sceneCacheTrack.IsAutoActivateObject())
            DeactivateSceneCacheInTrack(sceneCacheTrack);
    }

//--------------------------------------------------------------------------------------------------------------------------------------------------------------    

    //true if the track was muted, false otherwise
    private bool WasMuted(TrackAsset track) {
        //check if initialized
        if (!m_lastTrackMuted.TryGetValue(track, out bool lastMuted)) {
            m_lastTrackMuted[track] = track.muted;
            return false;
        }

        if (lastMuted == track.muted)
            return false;

        m_lastTrackMuted[track] = track.muted;
        return track.muted;
    }

    private static void DeactivateSceneCacheInTrack(SceneCacheTrack track) {
        Assert.IsNotNull(track);

        foreach (TimelineClip clip in track.GetClips()) {
            SceneCachePlayableAsset sceneCachePlayableAsset = clip.asset as SceneCachePlayableAsset;
            Assert.IsNotNull(sceneCachePlayableAsset);

            SceneCachePlayer scPlayer = sceneCachePlayableAsset.GetSceneCachePlayer();
            if (null == scPlayer)
                continue;

            scPlayer.gameObject.SetActive(false);
        }
    }

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

    private readonly Dictionary<TrackAsset, bool> m_lastTrackMuted = new Dictionary<TrackAsset, bool>();
}
}