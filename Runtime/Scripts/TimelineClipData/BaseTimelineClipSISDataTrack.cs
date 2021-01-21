using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.StreamingImageSequence { 
/// <summary>
/// A track which requires its TimelineClip to store BaseTimelineClipData as an extension
/// </summary>
internal abstract class BaseTimelineClipSISDataTrack<T> : BaseSISTrack where T: BaseTimelineClipDataPlayableAsset {
   
    
    
//----------------------------------------------------------------------------------------------------------------------
    /// <inheritdoc/>
    protected override void OnBeforeTrackSerialize() {
        base.OnBeforeTrackSerialize();
        m_serializedSISDataCollection = new List<BaseTimelineClipData>();
        
        foreach (TimelineClip clip in GetClips()) {
            BaseTimelineClipData data = null;

            if (null != m_sisDataCollection && m_sisDataCollection.ContainsKey(clip)) {                
                data =   m_sisDataCollection[clip];
            } else {                
                T sisPlayableAsset = clip.asset as T;
                Assert.IsNotNull(sisPlayableAsset);                 
                data = sisPlayableAsset.GetBoundTimelineClipData();
            }

            if (null == data) {
                data = new BaseTimelineClipData(clip);
            }
            
                       
            m_serializedSISDataCollection.Add(data);
        }
    }

    /// <inheritdoc/>
    protected override  void OnAfterTrackDeserialize() {
        base.OnAfterTrackDeserialize();
        m_sisDataCollection = new Dictionary<TimelineClip, BaseTimelineClipData>();
        
        
        IEnumerator<TimelineClip> clipEnumerator = GetClips().GetEnumerator();
        List<BaseTimelineClipData>.Enumerator sisEnumerator = m_serializedSISDataCollection.GetEnumerator();
        while (clipEnumerator.MoveNext() && sisEnumerator.MoveNext()) {
            TimelineClip clip = clipEnumerator.Current;
            Assert.IsNotNull(clip);

            BaseTimelineClipData sceneCacheClipData = sisEnumerator.Current;
            Assert.IsNotNull(sceneCacheClipData);           
            
            m_sisDataCollection[clip] = sceneCacheClipData;
            
        }
        clipEnumerator.Dispose();
        sisEnumerator.Dispose();
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    /// <inheritdoc/>
    public sealed override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount) {
               
        if (null == m_sisDataCollection) {
            m_sisDataCollection = new Dictionary<TimelineClip, BaseTimelineClipData>();
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
    private BaseTimelineClipData GetOrCreateTimelineClipSISData(TimelineClip clip) {
        Assert.IsNotNull(m_sisDataCollection);
        
        if (m_sisDataCollection.ContainsKey(clip)) {
            return m_sisDataCollection[clip];            
        }

        BaseTimelineClipData data = new BaseTimelineClipData(clip);
        m_sisDataCollection[clip] = data;
        return data;
    }
        
//----------------------------------------------------------------------------------------------------------------------
    private void InitTimelineClipSISData() {
        //Initialize PlayableAssets and BaseTimelineClipData       
        foreach (TimelineClip clip in GetClips()) {
            T sisPlayableAsset = clip.asset as T;
            Assert.IsNotNull(sisPlayableAsset);               
            
            BaseTimelineClipData sceneCacheClipData = sisPlayableAsset.GetBoundTimelineClipData();
            if (null == sceneCacheClipData) {
                sceneCacheClipData = GetOrCreateTimelineClipSISData(clip);                
            } else {
                if (!m_sisDataCollection.ContainsKey(clip)) {
                    m_sisDataCollection.Add(clip, sceneCacheClipData);;            
                }                
            }
            
            //Make sure that the clip is the owner
            sceneCacheClipData.SetOwner(clip);
            sisPlayableAsset.BindTimelineClipData(sceneCacheClipData);            
        }
        
    }


    
//----------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] List<BaseTimelineClipData> m_serializedSISDataCollection = null;

    private Dictionary<TimelineClip, BaseTimelineClipData> m_sisDataCollection = null;
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

