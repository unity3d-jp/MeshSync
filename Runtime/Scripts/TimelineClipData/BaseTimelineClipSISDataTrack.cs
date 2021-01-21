using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.StreamingImageSequence { 
/// <summary>
/// A track which requires its TimelineClip to store SceneCacheClipData as an extension
/// </summary>
internal abstract class BaseTimelineClipSISDataTrack<T> : BaseSISTrack where T: BaseTimelineClipDataPlayableAsset {
   
    
    
//----------------------------------------------------------------------------------------------------------------------
    /// <inheritdoc/>
    protected override void OnBeforeTrackSerialize() {
        base.OnBeforeTrackSerialize();
        m_serializedSISDataCollection = new List<SceneCacheClipData>();
        
        foreach (TimelineClip clip in GetClips()) {
            SceneCacheClipData data = null;

            if (null != m_sisDataCollection && m_sisDataCollection.ContainsKey(clip)) {                
                data =   m_sisDataCollection[clip];
            } else {                
                T sisPlayableAsset = clip.asset as T;
                Assert.IsNotNull(sisPlayableAsset);                 
                data = sisPlayableAsset.GetBoundTimelineClipSISData();
            }

            if (null == data) {
                data = new SceneCacheClipData(clip);
            }
            
                       
            m_serializedSISDataCollection.Add(data);
        }
    }

    /// <inheritdoc/>
    protected override  void OnAfterTrackDeserialize() {
        base.OnAfterTrackDeserialize();
        m_sisDataCollection = new Dictionary<TimelineClip, SceneCacheClipData>();
        
        
        IEnumerator<TimelineClip> clipEnumerator = GetClips().GetEnumerator();
        List<SceneCacheClipData>.Enumerator sisEnumerator = m_serializedSISDataCollection.GetEnumerator();
        while (clipEnumerator.MoveNext() && sisEnumerator.MoveNext()) {
            TimelineClip clip = clipEnumerator.Current;
            Assert.IsNotNull(clip);

            SceneCacheClipData sceneCacheClipData = sisEnumerator.Current;
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
            m_sisDataCollection = new Dictionary<TimelineClip, SceneCacheClipData>();
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
    private SceneCacheClipData GetOrCreateTimelineClipSISData(TimelineClip clip) {
        Assert.IsNotNull(m_sisDataCollection);
        
        if (m_sisDataCollection.ContainsKey(clip)) {
            return m_sisDataCollection[clip];            
        }

        SceneCacheClipData data = new SceneCacheClipData(clip);
        m_sisDataCollection[clip] = data;
        return data;
    }
        
//----------------------------------------------------------------------------------------------------------------------
    private void InitTimelineClipSISData() {
        //Initialize PlayableAssets and SceneCacheClipData       
        foreach (TimelineClip clip in GetClips()) {
            T sisPlayableAsset = clip.asset as T;
            Assert.IsNotNull(sisPlayableAsset);               
            
            SceneCacheClipData sceneCacheClipData = sisPlayableAsset.GetBoundTimelineClipSISData();
            if (null == sceneCacheClipData) {
                sceneCacheClipData = GetOrCreateTimelineClipSISData(clip);                
            } else {
                if (!m_sisDataCollection.ContainsKey(clip)) {
                    m_sisDataCollection.Add(clip, sceneCacheClipData);;            
                }                
            }
            
            //Make sure that the clip is the owner
            sceneCacheClipData.SetOwner(clip);
            sisPlayableAsset.BindTimelineClipSISData(sceneCacheClipData);            
        }
        
    }


    
//----------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] List<SceneCacheClipData> m_serializedSISDataCollection = null;

    private Dictionary<TimelineClip, SceneCacheClipData> m_sisDataCollection = null;
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

