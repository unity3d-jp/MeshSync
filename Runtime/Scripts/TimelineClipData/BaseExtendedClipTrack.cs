﻿using UnityEngine.Playables;
using UnityEngine.Timeline;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.FilmInternalUtilities { 
/// <summary>
/// A track which requires its TimelineClip to store BaseClipData as an extension
/// </summary>
internal abstract class BaseExtendedClipTrack<P,D> : BaseTrack 
    where P: BaseExtendedClipPlayableAsset
    where D: BaseClipData, new()
{
   
    
    
//----------------------------------------------------------------------------------------------------------------------
    /// <inheritdoc/>
    protected override void OnBeforeTrackSerialize() {
        base.OnBeforeTrackSerialize();
        m_serializedDataCollection = new List<BaseClipData>();
        
        foreach (TimelineClip clip in GetClips()) {
            BaseClipData data = null;

            if (null != m_dataCollection && m_dataCollection.ContainsKey(clip)) {                
                data =   m_dataCollection[clip];
            } else {                
                P sisPlayableAsset = clip.asset as P;
                Assert.IsNotNull(sisPlayableAsset);                 
                data = sisPlayableAsset.GetBoundClipData();
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
        m_dataCollection = new Dictionary<TimelineClip, BaseClipData>();

        if (null == m_serializedDataCollection) {
            m_serializedDataCollection = new List<BaseClipData>();
        }         
        
        IEnumerator<TimelineClip> clipEnumerator = GetClips().GetEnumerator();
        List<BaseClipData>.Enumerator sisEnumerator = m_serializedDataCollection.GetEnumerator();
        while (clipEnumerator.MoveNext() && sisEnumerator.MoveNext()) {
            TimelineClip clip = clipEnumerator.Current;
            Assert.IsNotNull(clip);

            BaseClipData sceneCacheClipData = sisEnumerator.Current;
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
            m_dataCollection = new Dictionary<TimelineClip, BaseClipData>();
        }
        InitClipData();
        Playable mixer = CreateTrackMixerInternal(graph, go, inputCount);
        
        return mixer;
    }

    protected abstract Playable CreateTrackMixerInternal(PlayableGraph graph, GameObject go, int inputCount);
    

//----------------------------------------------------------------------------------------------------------------------

    /// <inheritdoc/>
    public override string ToString() { return name; }   

        
//----------------------------------------------------------------------------------------------------------------------
    private void InitClipData() {
        //Initialize PlayableAssets and BaseClipData       
        foreach (TimelineClip clip in GetClips()) {
            P sisPlayableAsset = clip.asset as P;
            Assert.IsNotNull(sisPlayableAsset);

            //Try to get existing one, either from the collection, or the clip
            BaseClipData clipData = null;
            if (m_dataCollection.ContainsKey(clip)) {
                clipData = m_dataCollection[clip];            
            } else {
                clipData = sisPlayableAsset.GetBoundClipData();
            }

            if (null == clipData) {
                clipData = new D();                
            }
            
            //Fix the required data structure
            m_dataCollection[clip] = clipData;
            clipData.SetOwner(clip);
            sisPlayableAsset.BindClipData(clipData);                        
        }
        
    }


    
//----------------------------------------------------------------------------------------------------------------------

    [HideInInspector][SerializeField] List<BaseClipData> m_serializedDataCollection = null;

    private Dictionary<TimelineClip, BaseClipData> m_dataCollection = null;
    
}

} //end namespace

//----------------------------------------------------------------------------------------------------------------------

