using System.IO;
using UnityEditor;

namespace Unity.MeshSync.Editor {

class SceneCachePostprocessor : AssetPostprocessor {
    static void OnPostprocessAllAssets(string[] importedAssets, string[] deletedAssets, string[] movedAssets, string[] movedFromAssetPaths) {
        foreach (string movedAssetPath in movedAssets) {
            string ext = Path.GetExtension(movedAssetPath);
            if (ext != ".sc")
                continue;
            
            AssetDatabase.ImportAsset(movedAssetPath);
        }
    }
}


} //end namespace
