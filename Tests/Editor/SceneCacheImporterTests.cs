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
        string destPath = AssetDatabase.GenerateUniqueAssetPath("Assets/Copied.sc");
        File.Copy(MeshSyncTestEditorConstants.CUBE_TEST_DATA_PATH, destPath);
        AssetDatabase.ImportAsset(destPath);
        yield return null;

        SceneCachePlayer player = AssetDatabase.LoadAssetAtPath<SceneCachePlayer>(destPath);
        Assert.IsNotNull(player);
        yield return null;
        

        AssetDatabase.DeleteAsset(destPath);
        
    }
    
   
}

}