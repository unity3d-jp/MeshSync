using System;
using UnityEngine;

namespace Unity.MeshSync {
    
[Serializable]
internal class PlayableFrameProperty<T> where T: struct {
    
    internal PlayableFrameProperty(KeyFramePropertyID id, T val) {
        m_propertyID  = id;
        m_propertyValue = val;
    }
//----------------------------------------------------------------------------------------------------------------------

    internal KeyFramePropertyID GetID() { return m_propertyID; }
    internal T GetValue() { return m_propertyValue;}

//----------------------------------------------------------------------------------------------------------------------


    [SerializeField] private KeyFramePropertyID m_propertyID;
    [SerializeField] private T m_propertyValue = default(T);
    
}

} //end namespace

