using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.StreamingImageSequence { 
/// <summary>
/// A track which requires its TimelineClip to store TimelineClipData as an extension
/// </summary>
internal abstract class BaseTimelineClipSISDataTrack<T> : BaseSISTrack where T: BaseTimelineClipDataPlayableAsset {
   
    
    
//----------------------------------------------------------------------------------------------------------------------
    /// <inheritdoc/>
    protected override void OnBeforeTrackSerialize() {
        base.OnBeforeTrackSerialize();
        m_serializedSISDataCollection = new List<TimelineClipData>();
        
        foreach (TimelineClip clip in GetClips()) {
            TimelineClipData data = null;

            if (null != m_sisDataCollection && m_sisDataCollection.ContainsKey(clip)) {                
                data =   m_sisDataCollection[clip];
            } else {                
                T sisPlayableAsset = clip.asset as T;
                Assert.IsNotNull(sisPlayableAsset);                 
                data = sisPlayableAsset.GetBoundTimelineClipSISData();
            }

            if (null == data) {
                data = new TimelineClipData(clip);
            }
            
                       
            m_serializedSISDataCollection.Add(data);
        }
    }

    /// <inheritdoc/>
    protected override  void OnAfterTrackDeserialize() {
        base.OnAfterTrackDeserialize();
        m_sisDataCollection = new Dictionary<TimelineClip, TimelineClipData>();
        
        
        IEnumerator<TimelineClip> clipEnumerator = GetClips().GetEnumerator();
        List<TimelineClipData>.Enumerator sisEnumerator = m_serializedSISDataCollection.GetEnumerator();
        while (clipEnumerator.MoveNext() && sisEnumerator.MoveNext()) {
            TimelineClip clip = clipEnumerator.Current;
            Assert.IsNotNull(clip);

            TimelineClipData timelineClipData = sisEnumerator.Current;
            Assert.IsNotNull(timelineClipData);           
            
            m_sisDataCollection[clip] = timelineClipData;
            
        }
        clipEnumerator.Dispose();
        sisEnumerator.Dispose();
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    /// <inheritdoc/>
    public sealed override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount) {
               
        if (null == m_sisDataCollection) {
            m_sisDataCollection = new Dictionary<TimelineClip, TimelineClipData>();
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
    private TimelineClipData GetOrCreateTimelineClipSISData(TimelineClip clip) {
        Assert.IsNotNull(m_sisDataCollection);
        
        if (m_sisDataCollection.ContainsKey(clip)) {
            return m_sisDataCollection[clip];            
        }

        TimelineClipData data = new TimelineClipData(clip);
        m_sisDataCollection[clip] = data;
        return data;
    }
        
//----------------------------------------------------------------------------------------------------------------------
    private void InitTimelineClipSISData() {
        //Initialize PlayableAssets and TimelineClipData       
        foreach (TimelineClip clip in GetClips()) {
            T sisPlayableAsset = clip.asset as T;
            Assert.IsNotNull(sisPlayableAsset);               
            
            TimelineClipData timelineClipData = sisPlayableAsset.GetBoundTimelineClipSISData();
            if (null == timelineClipData) {
                timelineClipData = GetOrCreateTimelineClipSISData(clip);                
            } else {
                if (!m_sisDataCollection.ContainsKey(clip)) {
                    m_sisDataCollection.Add(clip, timelineClipData);;            
                }                
            }
            
            //Make sure that the clip is the owner
            timelineClipData.SetOwner(clip);
            sisPlayableAsset.BindTimelineClipSISData(timelineClipData);            
        }
        
    }


    
//----------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] List<TimelineClipData> m_serializedSISDataCollection = null;

    private Dictionary<TimelineClip, TimelineClipData> m_sisDataCollection = null;
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

