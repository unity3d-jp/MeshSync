using System.IO;
using NUnit.Framework;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor.Tests {


public class SceneCachePlayerTest  {

    [Test]
    public void CreateSceneCache() {

        string cubeTestDataPath = Path.Combine(TEST_DATA_PATH, "Cube.sc");
        string sphereTestDataPath = Path.Combine(TEST_DATA_PATH, "Sphere.sc");

        CreateAndDeleteSceneCachePlayerPrefab(cubeTestDataPath);
        CreateAndDeleteSceneCachePlayerPrefab(sphereTestDataPath);
        
    }

//----------------------------------------------------------------------------------------------------------------------    

    private void CreateAndDeleteSceneCachePlayerPrefab(string sceneCachePath) {
        Assert.IsTrue(File.Exists(sceneCachePath));
        
        
        GameObject go = SceneCachePlayerEditor.CreateSceneCachePlayerPrefab(Path.GetFullPath(sceneCachePath));
        Assert.IsNotNull(go);
        GameObject prefab = PrefabUtility.GetCorrespondingObjectFromSource(go);
        Assert.IsNotNull(prefab);
        string prefabPath = AssetDatabase.GetAssetPath(prefab);
        Assert.IsFalse(string.IsNullOrEmpty(prefabPath));

        string prefabAssetsFolder = Path.Combine(
            Path.GetDirectoryName(prefabPath),
            Path.GetFileNameWithoutExtension(prefabPath)
        );
        
        Assert.IsFalse(string.IsNullOrEmpty(prefabAssetsFolder));       
        
        string[] prefabAssetGUIDs = AssetDatabase.FindAssets("", new[] {prefabAssetsFolder});
        foreach (string guid in prefabAssetGUIDs) {
            AssetDatabase.DeleteAsset(AssetDatabase.GUIDToAssetPath(guid));
        }

        FileUtil.DeleteFileOrDirectory(prefabAssetsFolder);
        
        //Cleanup
        Object.DestroyImmediate(go);
        AssetDatabase.DeleteAsset(prefabPath);

        AssetDatabase.Refresh();
        
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    private readonly string TEST_DATA_PATH = Path.Combine("Packages", MeshSyncConstants.PACKAGE_NAME, "Tests", "Data");

}

}