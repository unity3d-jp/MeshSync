using System.Collections;
using System.IO;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.TestTools;

namespace Unity.MeshSync.Editor.Tests {


internal class SceneCacheImporterTests  {

    [UnityTest]
    public IEnumerator MoveSceneCacheFile() {
                
        //Initial setup
        string initialPath = AssetDatabase.GenerateUniqueAssetPath("Assets/Copied.sc");
        File.Copy(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH, initialPath);
        AssetDatabase.ImportAsset(initialPath);
        yield return null;

        SceneCachePlayer player = AssetDatabase.LoadAssetAtPath<SceneCachePlayer>(initialPath);        
        Assert.IsNotNull(player);
        Assert.AreEqual(initialPath, player.GetSceneCacheFilePath());
        yield return null;

        string movedPath = AssetDatabase.GenerateUniqueAssetPath("Assets/Moved.sc");
        AssetDatabase.MoveAsset(initialPath, movedPath);
        yield return null;
        
        player = AssetDatabase.LoadAssetAtPath<SceneCachePlayer>(movedPath);
        Assert.AreEqual(movedPath, player.GetSceneCacheFilePath());
        yield return null;
        
        AssetDatabase.DeleteAsset(movedPath);
    }
}

} //end namespace