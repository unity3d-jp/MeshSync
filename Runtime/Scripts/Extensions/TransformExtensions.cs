using System.Collections.Generic;
using UnityEngine;

namespace Unity.MeshSync {

internal static class TransformExtensions {

    //[TODO-sin: 2021-9-6] Move to FIU
    internal static Transform FindOrCreate(this Transform t, string childName, bool worldPositionStays = true) {
        
        Transform childT = t.Find(childName);
        if (null != childT)
            return childT;
                
        GameObject go = new GameObject(childName);
        childT = go.transform;
        childT.SetParent(t, worldPositionStays);
        return childT;
    }
    

    //[TODO-sin: 2021-10-18] Move to FIU 
    internal static void SetParent(this ICollection<Transform> collection, Transform parent) {        
        foreach (Transform t in collection) {
            t.SetParent(parent);
        }
    }
    
}

} //end namespace