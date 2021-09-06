using System;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Unity.MeshSync {

internal static class ObjectUtility {

    //[TODO-sin: 2021-9-6] Move to FIU
    internal static Transform FindFirstRootGameObject(string objectName) {
        Transform ret = null;

        GameObject[] roots = SceneManager.GetActiveScene().GetRootGameObjects();
        foreach (GameObject go in roots) {
            if (go.name != objectName) 
                continue;
                
            ret = go.GetComponent<Transform>();
            break;
        }
        return ret;
    }
    
}

} //end namespace