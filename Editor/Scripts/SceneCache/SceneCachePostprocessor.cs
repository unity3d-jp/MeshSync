using System.IO;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal class SceneCachePostprocessor : AssetPostprocessor {
    private static void OnPostprocessAllAssets(string[] importedAssets, string[] deletedAssets, string[] movedAssets, string[] movedFromAssetPaths) {
        foreach (string movedAssetPath in movedAssets) {
            string ext = Path.GetExtension(movedAssetPath);
            if (ext != DOT_SCENE_CACHE_EXTENSION)
                continue;

            //check if we need to reimport to ensure that sceneCacheFilePath points to the new one
            SceneCachePlayer player = AssetDatabase.LoadAssetAtPath<SceneCachePlayer>(movedAssetPath);
            if (player.GetSceneCacheFilePath() == movedAssetPath)
                continue;

            AssetDatabase.ImportAsset(movedAssetPath);
        }
    }

    private static readonly string DOT_SCENE_CACHE_EXTENSION = $".{MeshSyncEditorConstants.SCENE_CACHE_EXTENSION}";
}
} //end namespace