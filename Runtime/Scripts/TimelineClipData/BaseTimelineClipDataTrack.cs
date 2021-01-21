﻿using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.FilmInternalUtilities { 
/// <summary>
/// A track which requires its TimelineClip to store BaseTimelineClipData as an extension
/// </summary>
internal abstract class BaseTimelineClipDataTrack<P,D> : BaseFilmTrack 
    where P: BaseTimelineClipDataPlayableAsset
    where D: BaseTimelineClipData, new()
{
   
    
    
//----------------------------------------------------------------------------------------------------------------------
    /// <inheritdoc/>
    protected override void OnBeforeTrackSerialize() {
        base.OnBeforeTrackSerialize();
        m_serializedDataCollection = new List<BaseTimelineClipData>();
        
        foreach (TimelineClip clip in GetClips()) {
            BaseTimelineClipData data = null;

            if (null != m_dataCollection && m_dataCollection.ContainsKey(clip)) {                
                data =   m_dataCollection[clip];
            } else {                
                P sisPlayableAsset = clip.asset as P;
                Assert.IsNotNull(sisPlayableAsset);                 
                data = sisPlayableAsset.GetBoundTimelineClipData();
            }

            if (null == data) {
                data = new D();
                data.SetOwner(clip);
            }
            
                       
            m_serializedDataCollection.Add(data);
        }
    }

    /// <inheritdoc/>
    protected override  void OnAfterTrackDeserialize() {
        base.OnAfterTrackDeserialize();
        m_dataCollection = new Dictionary<TimelineClip, BaseTimelineClipData>();
        
        
        IEnumerator<TimelineClip> clipEnumerator = GetClips().GetEnumerator();
        List<BaseTimelineClipData>.Enumerator sisEnumerator = m_serializedDataCollection.GetEnumerator();
        while (clipEnumerator.MoveNext() && sisEnumerator.MoveNext()) {
            TimelineClip clip = clipEnumerator.Current;
            Assert.IsNotNull(clip);

            BaseTimelineClipData sceneCacheClipData = sisEnumerator.Current;
            Assert.IsNotNull(sceneCacheClipData);           
            
            m_dataCollection[clip] = sceneCacheClipData;
            
        }
        clipEnumerator.Dispose();
        sisEnumerator.Dispose();
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    /// <inheritdoc/>
    public sealed override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount) {
               
        if (null == m_dataCollection) {
            m_dataCollection = new Dictionary<TimelineClip, BaseTimelineClipData>();
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
        Assert.IsNotNull(m_dataCollection);
        
        if (m_dataCollection.ContainsKey(clip)) {
            return m_dataCollection[clip];            
        }

        BaseTimelineClipData data = new D();
        data.SetOwner(clip);
        m_dataCollection[clip] = data;
        return data;
    }
        
//----------------------------------------------------------------------------------------------------------------------
    private void InitTimelineClipSISData() {
        //Initialize PlayableAssets and BaseTimelineClipData       
        foreach (TimelineClip clip in GetClips()) {
            P sisPlayableAsset = clip.asset as P;
            Assert.IsNotNull(sisPlayableAsset);               
            
            BaseTimelineClipData sceneCacheClipData = sisPlayableAsset.GetBoundTimelineClipData();
            if (null == sceneCacheClipData) {
                sceneCacheClipData = GetOrCreateTimelineClipSISData(clip);                
            } else {
                if (!m_dataCollection.ContainsKey(clip)) {
                    m_dataCollection.Add(clip, sceneCacheClipData);;            
                }                
            }
            
            //Make sure that the clip is the owner
            sceneCacheClipData.SetOwner(clip);
            sisPlayableAsset.BindTimelineClipData(sceneCacheClipData);            
        }
        
    }


    
//----------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] List<BaseTimelineClipData> m_serializedDataCollection = null;

    private Dictionary<TimelineClip, BaseTimelineClipData> m_dataCollection = null;
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

