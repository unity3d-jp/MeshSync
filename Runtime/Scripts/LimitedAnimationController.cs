using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class LimitedAnimationController {

    internal LimitedAnimationController() { }

    internal LimitedAnimationController(bool enabled, int numFramesToHold, int frameOffset) {
        m_enabled         = enabled;
        m_numFramesToHold = numFramesToHold;
        m_frameOffset     = frameOffset;
    }    
    
//----------------------------------------------------------------------------------------------------------------------
    internal void SetEnabled(bool enabled) { m_enabled = enabled;}
    internal bool IsEnabled()              { return m_enabled;}

    internal void SetNumFramesToHold(int numFrames) { m_numFramesToHold = numFrames;}
    internal int  GetNumFramesToHold()              { return m_numFramesToHold;}

    internal void SetFrameOffset(int offset) {
        m_frameOffset = Mathf.Clamp(offset,0,m_numFramesToHold-1);
    }
    internal int  GetFrameOffset()            { return m_frameOffset;}
    
//----------------------------------------------------------------------------------------------------------------------
    
    internal int Apply(int frame) {
        if (!m_enabled) {
            return frame;
        }

        m_numFramesToHold = Mathf.Max(1, m_numFramesToHold);
        int multiplier = Mathf.FloorToInt((float) (Mathf.Max(0,frame - m_frameOffset)) / m_numFramesToHold);
        frame = (multiplier * m_numFramesToHold) + m_frameOffset;
        return frame;
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    [SerializeField] private bool m_enabled         = false;
    [SerializeField] private int  m_numFramesToHold = 1; //hold one data for several frames.    
    [SerializeField] private int  m_frameOffset = 0; 
    
}    
    
} //end namespace