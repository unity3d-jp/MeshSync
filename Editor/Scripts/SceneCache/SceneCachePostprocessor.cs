using System.IO;
using UnityEditor;

namespace Unity.MeshSync.Editor {

class SceneCachePostprocessor : AssetPostprocessor {
    static void OnPostprocessAllAssets(string[] importedAssets, string[] deletedAssets, string[] movedAssets, string[] movedFromAssetPaths) {
        
        foreach (string movedAssetPath in movedAssets) {
            string ext = Path.GetExtension(movedAssetPath);
            if (ext != DOT_SCENE_CACHE_EXTENSION)
                continue;
            
            AssetDatabase.ImportAsset(movedAssetPath); //need to reimport to ensure that sceneCacheFilePath points to the new one
        }
    }

    private static readonly string DOT_SCENE_CACHE_EXTENSION = $".{MeshSyncEditorConstants.SCENE_CACHE_EXTENSION}";
}


} //end namespace
