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
    
    internal int Apply(int frame) {
        if (!m_enabled) {
            return frame;
        }

        m_numFramesToHold = Mathf.Max(1, m_numFramesToHold);
        int multiplier = Mathf.FloorToInt((float) frame / m_numFramesToHold);
        frame = multiplier * m_numFramesToHold;
        return frame;
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    [SerializeField] private bool m_enabled         = false;
    [SerializeField] private int  m_numFramesToHold = 1; //hold one data for several frames.
    [SerializeField] private int  m_frameOffset = 0; 
    
}    
    
} //end namespace