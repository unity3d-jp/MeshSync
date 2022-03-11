using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class LimitedAnimationConfig {

    internal void SetEnabled(bool enabled) { m_enabled = enabled;}
    internal bool IsEnabled()              { return m_enabled;}

    internal void SetNumFramesToHold(int numFrames) { m_numFramesToHold = numFrames;}
    internal int  GetNumFramesToHold()              { return m_numFramesToHold;}

    internal void SetFrameOffset(int offset ) { m_frameOffset = offset;}
    internal int  GetFrameOffset()            { return m_frameOffset;}
    
//----------------------------------------------------------------------------------------------------------------------    
    [SerializeField] private bool m_enabled         = false;
    [SerializeField] private int  m_numFramesToHold = 1; //hold one data for several frames.
    [SerializeField] private int  m_frameOffset = 0; 
    
}    
    
} //end namespace