using System.IO;
using UnityEditor;

namespace Unity.MeshSync.Editor {

class SceneCachePostprocessor : AssetPostprocessor {
    static void OnPostprocessAllAssets(string[] importedAssets, string[] deletedAssets, string[] movedAssets, string[] movedFromAssetPaths) {
        
        foreach (string movedAssetPath in movedAssets) {
            string ext = Path.GetExtension(movedAssetPath);
            if (ext != DOT_SCENE_CACHE_EXTENSION)
                continue;
            
            AssetDatabase.ImportAsset(movedAssetPath);
        }
    }

    private static readonly string DOT_SCENE_CACHE_EXTENSION = $".{MeshSyncEditorConstants.SCENE_CACHE_EXTENSION}";
}


} //end namespace
