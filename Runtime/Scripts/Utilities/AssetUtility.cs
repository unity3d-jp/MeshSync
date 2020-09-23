using UnityEngine;

namespace Unity.MeshSync {

internal static class AssetUtility {


    //[TODO-sin: 2020-9-18: Move to AnimeToolbox]
    //Normalize so that the path is relative to the Unity root project
    public static string NormalizeAssetPath(string path) {
        if (string.IsNullOrEmpty(path))
            return null;

        if (path.StartsWith(Application.dataPath)) {
            return path.Substring(Application.dataPath.Length - "Assets".Length);
        }
        return path;
    }

}

} //end namespace