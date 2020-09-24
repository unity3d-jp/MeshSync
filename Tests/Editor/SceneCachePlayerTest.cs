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
        
        
        GameObject go = new GameObject();
        bool prefabCreated = MeshSyncMenu.CreateSceneCachePlayerPrefab(go, Path.GetFullPath(sceneCachePath));
        Assert.IsTrue(prefabCreated);
        GameObject prefab = PrefabUtility.GetCorrespondingObjectFromSource(go);
        Assert.IsNotNull(prefab);
        string prefabPath = AssetDatabase.GetAssetPath(prefab);
        Assert.IsFalse(string.IsNullOrEmpty(prefabPath));

        string prefabDir = Path.GetDirectoryName(prefabPath);
        Assert.IsNotNull(prefabDir);

        string prefabAssetsFolder = Path.Combine(
            prefabDir,
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