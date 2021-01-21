using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.StreamingImageSequence { 
/// <summary>
/// A track which requires its TimelineClip to store TimelineClipSISData as an extension
/// </summary>
internal abstract class BaseTimelineClipSISDataTrack<T> : BaseSISTrack where T: BaseTimelineClipSISDataPlayableAsset {
   
    
    
//----------------------------------------------------------------------------------------------------------------------
    /// <inheritdoc/>
    protected override void OnBeforeTrackSerialize() {
        base.OnBeforeTrackSerialize();
        m_serializedSISDataCollection = new List<TimelineClipSISData>();
        
        foreach (TimelineClip clip in GetClips()) {
            TimelineClipSISData sisData = null;

            if (null != m_sisDataCollection && m_sisDataCollection.ContainsKey(clip)) {                
                sisData =   m_sisDataCollection[clip];
            } else {                
                T sisPlayableAsset = clip.asset as T;
                Assert.IsNotNull(sisPlayableAsset);                 
                sisData = sisPlayableAsset.GetBoundTimelineClipSISData();
            }

            if (null == sisData) {
                sisData = new TimelineClipSISData(clip);
            }
            
                       
            m_serializedSISDataCollection.Add(sisData);
        }
    }

    /// <inheritdoc/>
    protected override  void OnAfterTrackDeserialize() {
        base.OnAfterTrackDeserialize();
        m_sisDataCollection = new Dictionary<TimelineClip, TimelineClipSISData>();
        
        
        IEnumerator<TimelineClip> clipEnumerator = GetClips().GetEnumerator();
        List<TimelineClipSISData>.Enumerator sisEnumerator = m_serializedSISDataCollection.GetEnumerator();
        while (clipEnumerator.MoveNext() && sisEnumerator.MoveNext()) {
            TimelineClip clip = clipEnumerator.Current;
            Assert.IsNotNull(clip);

            TimelineClipSISData timelineClipSISData = sisEnumerator.Current;
            Assert.IsNotNull(timelineClipSISData);           
            
            m_sisDataCollection[clip] = timelineClipSISData;
            
        }
        clipEnumerator.Dispose();
        sisEnumerator.Dispose();
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    /// <inheritdoc/>
    public sealed override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount) {
               
        if (null == m_sisDataCollection) {
            m_sisDataCollection = new Dictionary<TimelineClip, TimelineClipSISData>();
        }
        InitTimelineClipSISData();
        Playable mixer = CreateTrackMixerInternal(graph, go, inputCount);
        
        return mixer;
    }

    protected abstract Playable CreateTrackMixerInternal(PlayableGraph graph, GameObject go, int inputCount);
    

//----------------------------------------------------------------------------------------------------------------------

    /// <inheritdoc/>
    public override string ToString() { return name; }
    

//----------------------------------------------------------------------------------------------------------------------
    private TimelineClipSISData GetOrCreateTimelineClipSISData(TimelineClip clip) {
        Assert.IsNotNull(m_sisDataCollection);
        
        if (m_sisDataCollection.ContainsKey(clip)) {
            return m_sisDataCollection[clip];            
        }

        TimelineClipSISData sisData = new TimelineClipSISData(clip);
        m_sisDataCollection[clip] = sisData;
        return sisData;
    }
        
//----------------------------------------------------------------------------------------------------------------------
    private void InitTimelineClipSISData() {
        //Initialize PlayableAssets and TimelineClipSISData       
        foreach (TimelineClip clip in GetClips()) {
            T sisPlayableAsset = clip.asset as T;
            Assert.IsNotNull(sisPlayableAsset);               
            
            TimelineClipSISData timelineClipSISData = sisPlayableAsset.GetBoundTimelineClipSISData();
            if (null == timelineClipSISData) {
                timelineClipSISData = GetOrCreateTimelineClipSISData(clip);                
            } else {
                if (!m_sisDataCollection.ContainsKey(clip)) {
                    m_sisDataCollection.Add(clip, timelineClipSISData);;            
                }                
            }
            
            //Make sure that the clip is the owner
            timelineClipSISData.SetOwner(clip);
            sisPlayableAsset.BindTimelineClipSISData(timelineClipSISData);            
        }
        
    }


    
//----------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] List<TimelineClipSISData> m_serializedSISDataCollection = null;

    private Dictionary<TimelineClip, TimelineClipSISData> m_sisDataCollection = null;
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

