﻿using System;
using JetBrains.Annotations;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Unity.MeshSync {

internal static class TransformUtility {

    //[TODO-sin: 2021-9-6] Move to FIU
    [CanBeNull]
    internal static Transform FindFirstRoot(string objectName) {

        GameObject[] roots = SceneManager.GetActiveScene().GetRootGameObjects();
        foreach (GameObject go in roots) {
            if (go.name != objectName) 
                continue;

            return go.transform;
        }
        return null;
    }
    

    // returns null if not found
    private static Transform FindByPath(Transform parent, string path) {
        string[] names = path.Split('/');
        if (names.Length <= 0)
            return null;

        //if parent is null, search from root 
        Transform t = parent;
        int       tokenStartIdx = 0;
        if (null == t) {
            t = FindFirstRoot(names[0]);
            if (null == t)
                return t;

            tokenStartIdx = 1;
        }

        //loop
        int nameLength = names.Length;
        for (int i = tokenStartIdx; i < nameLength; ++i) {
            string nameToken = names[i];
            if (string.IsNullOrEmpty(nameToken))
                continue;

            t = t.Find(nameToken);
            if (null == t)
                return t;
        }

        return t;        
    }
    
    
    
}

} //end namespace