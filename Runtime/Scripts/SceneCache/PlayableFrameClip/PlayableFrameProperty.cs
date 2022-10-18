using System;
using UnityEngine;

namespace Unity.MeshSync {
    
[Serializable]
internal class PlayableFrameProperty{
    
    internal PlayableFrameProperty(PlayableFramePropertyID id, bool val) {
        m_propertyID  = id;
        m_propertyValue = val;
    }
//----------------------------------------------------------------------------------------------------------------------

    internal PlayableFramePropertyID GetID() { return m_propertyID; }
    internal bool GetValue() { return m_propertyValue;}

//----------------------------------------------------------------------------------------------------------------------


    [SerializeField] private PlayableFramePropertyID m_propertyID;
    [SerializeField] private bool m_propertyValue = false;
    
}

} //end namespace

