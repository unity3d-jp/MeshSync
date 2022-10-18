using System;
using UnityEngine;

namespace Unity.MeshSync {
    
[Serializable]
internal class PlayableFrameProperty<T> where T: struct {
    
    internal PlayableFrameProperty(PlayableFramePropertyID id, T val) {
        m_propertyID  = id;
        m_propertyValue = val;
    }
//----------------------------------------------------------------------------------------------------------------------

    internal PlayableFramePropertyID GetID() { return m_propertyID; }
    internal T GetValue() { return m_propertyValue;}

//----------------------------------------------------------------------------------------------------------------------


    [SerializeField] private PlayableFramePropertyID m_propertyID;
    [SerializeField] private T m_propertyValue = default(T);
    
}

} //end namespace

