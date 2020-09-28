using System.IO;
using NUnit.Framework;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor.Tests {


public class SceneCachePlayerTest  {

    [Test]
    public void CreateSceneCache() {
        CreateAndDeleteSceneCachePlayerPrefab(CUBE_TEST_DATA_PATH);
        CreateAndDeleteSceneCachePlayerPrefab(SPHERE_TEST_DATA_PATH);        
    }

//----------------------------------------------------------------------------------------------------------------------       
    [Test]
    public void ChangeSceneCache() {
        
        //Initial setup            
        GameObject       go     = new GameObject();
        SceneCachePlayer player = go.AddComponent<SceneCachePlayer>();
        Assert.IsFalse(player.IsSceneCacheOpened());
        
        //Change
        SceneCachePlayerInspector.ChangeSceneCacheFile(player, Path.GetFullPath(CUBE_TEST_DATA_PATH));
        Assert.IsTrue(player.IsSceneCacheOpened());       
        SceneCachePlayerInspector.ChangeSceneCacheFile(player, Path.GetFullPath(SPHERE_TEST_DATA_PATH));
        Assert.IsTrue(player.IsSceneCacheOpened());

        //Cleanup
        Object.DestroyImmediate(go);
        
    }
    
//----------------------------------------------------------------------------------------------------------------------       

    private void CreateAndDeleteSceneCachePlayerPrefab(string sceneCachePath) {
        Assert.IsTrue(File.Exists(sceneCachePath));

        const string DEST_PREFAB_PATH  = "Assets/TestSceneCache.prefab";
        const string ASSETS_FOLDER     = "Assets/TestSceneCacheAssets";
            
        bool  prefabCreated  = MeshSyncMenu.CreateSceneCachePlayerAndPrefab(
            Path.GetFullPath(sceneCachePath), DEST_PREFAB_PATH,ASSETS_FOLDER, 
            out SceneCachePlayer player, out GameObject prefab
        );
        Assert.IsTrue(player.IsSceneCacheOpened());       
        Assert.IsTrue(prefabCreated);
        Assert.IsNotNull(prefab);
        Assert.IsNotNull(player);
        
        //Check the prefab
        string prefabPath = AssetDatabase.GetAssetPath(prefab);
        Assert.IsFalse(string.IsNullOrEmpty(prefabPath));
        Assert.AreEqual(DEST_PREFAB_PATH, prefabPath);

        //Check assets folder
        Assert.IsTrue(Directory.Exists(ASSETS_FOLDER));
        string[] prefabAssetGUIDs = AssetDatabase.FindAssets("", new[] {ASSETS_FOLDER});
        Assert.Greater(prefabAssetGUIDs.Length, 0);
        
        
        //Cleanup
        foreach (string guid in prefabAssetGUIDs) {
            AssetDatabase.DeleteAsset(AssetDatabase.GUIDToAssetPath(guid));
        }
        FileUtil.DeleteFileOrDirectory(ASSETS_FOLDER);
        AssetDatabase.DeleteAsset(prefabPath);

        AssetDatabase.Refresh();
        
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    private static readonly string TEST_DATA_PATH = Path.Combine("Packages", MeshSyncConstants.PACKAGE_NAME, "Tests", "Data");
    private static readonly string CUBE_TEST_DATA_PATH   = Path.Combine(TEST_DATA_PATH, "Cube.sc");
    private static readonly string SPHERE_TEST_DATA_PATH = Path.Combine(TEST_DATA_PATH, "Sphere.sc");
    
}

}