using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {

internal static class AssetEditorUtility {

    //[TODO-sin: 2020-9-23] Move to anime-toolbox
    internal static bool PingAssetByPath(string path) {
        Object asset = AssetDatabase.LoadAssetAtPath<UnityEngine.Object>(AssetUtility.NormalizeAssetPath(path));
        if (asset == null) return 
            false;
        
        EditorGUIUtility.PingObject(asset);
        return true;        
    }
    
    
    
}

} //end namespace