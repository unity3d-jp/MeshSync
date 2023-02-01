using System.Collections;
using System.IO;
using NUnit.Framework;
using UnityEditor;
using UnityEngine.TestTools;

namespace Unity.MeshSync.Editor.Tests {
internal class SceneCacheImporterTests  {
    [UnityTest]
    public IEnumerator MoveSceneCacheFile() {
        const string INITIAL_FOLDER = "Assets/TestRunnerInitialFolder";
        const string MOVED_FOLDER   = "Assets/TestRunnerMovedFolder";

        //Delete folders for easy retesting 
        AssetDatabase.DeleteAsset(INITIAL_FOLDER);
        AssetDatabase.DeleteAsset(MOVED_FOLDER);

        AssetDatabase.CreateFolder("Assets", Path.GetFileName(INITIAL_FOLDER));
        string fileName = Path.GetFileName(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH);

        //Initial setup
        string initialPath = AssetDatabase.GenerateUniqueAssetPath($"{INITIAL_FOLDER}/{fileName}");
        AssetDatabase.CopyAsset(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH, initialPath);
        yield return null;

        SceneCachePlayer player = AssetDatabase.LoadAssetAtPath<SceneCachePlayer>(initialPath);
        Assert.IsNotNull(player);
        Assert.AreEqual(initialPath, player.GetSceneCacheFilePath());
        yield return null;

        AssetDatabase.MoveAsset(INITIAL_FOLDER, MOVED_FOLDER);
        yield return null;

        string movedPath = $"{MOVED_FOLDER}/{fileName}";
        Assert.IsTrue(File.Exists(movedPath));


        player = AssetDatabase.LoadAssetAtPath<SceneCachePlayer>(movedPath);
        Assert.AreEqual(movedPath, player.GetSceneCacheFilePath());
        yield return null;

        AssetDatabase.DeleteAsset(movedPath);
        AssetDatabase.DeleteAsset(MOVED_FOLDER);
    }
}
} //end namespace