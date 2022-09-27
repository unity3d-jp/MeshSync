using System.Collections.Generic;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Timeline;

namespace Unity.MeshSync.Editor {

[CustomTimelineEditor(typeof(SceneCacheTrack))]
internal class SceneCacheTrackEditor : TrackEditor {
    
    
    public override void OnCreate(TrackAsset track, TrackAsset copiedFrom)
    {
        Debug.Log("OnCreate : " + track +  " muted " + track.muted);
    }
    
    public override void OnTrackChanged(TrackAsset track) {

        if (WasMuted(track)) {
            Debug.Log("OnTrackChanged : " + track +  " muted " + track.muted);
            
        }
        
    }

    //true if the track was muted, false otherwise
    bool WasMuted(TrackAsset track) {

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


    private readonly Dictionary<TrackAsset, bool> m_lastTrackMuted = new Dictionary<TrackAsset, bool>();

}

}