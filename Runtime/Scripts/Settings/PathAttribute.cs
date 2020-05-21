using System;
using System.IO;
using Unity.AnimeToolbox;
using UnityEngine;


namespace Unity.MeshSync {

[AttributeUsage(AttributeTargets.Class)]
internal class PathAttribute : Attribute {
      
    public PathAttribute(string path) {
        if (string.IsNullOrEmpty(path)) {
            Debug.LogError("[MeshSync] path is null or empty.");
            return;
        }

        m_path = path;
    }

    internal string GetPath() { return m_path; }

    private readonly string m_path;
}


} //end namespace