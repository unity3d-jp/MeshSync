using System;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Unity.MeshSync {

internal static class ObjectUtility {

    //[TODO-sin: 2021-9-6] Move to FIU
    internal static GameObject FindFirstRootGameObject(string objectName) {

        GameObject[] roots = SceneManager.GetActiveScene().GetRootGameObjects();
        foreach (GameObject go in roots) {
            if (go.name != objectName) 
                continue;

            return go;
        }
        return null;
    }
    
}

} //end namespace