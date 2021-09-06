using UnityEngine;

namespace Unity.MeshSync {

internal static class TransformExtensions {

    //[TODO-sin: 2021-9-6] Move to FIU
    internal static Transform FindOrCreate(this Transform t, string childName, bool worldPositionStays = true) {
        
        Transform childT = t.Find(childName);
        if (null != childT)
            return childT;
                
        GameObject go = new GameObject { name = childName };
        childT = go.transform;
        childT.SetParent(t, worldPositionStays);
        return childT;
    }
}

} //end namespace