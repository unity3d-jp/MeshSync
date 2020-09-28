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
    public void ChangeSceneCacheOnGameObject() {
        
        //Initial setup            
        GameObject       go     = new GameObject();
        SceneCachePlayer player = go.AddComponent<SceneCachePlayer>();
        Assert.IsFalse(player.IsSceneCacheOpened());
        
        //Change
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(player, Path.GetFullPath(CUBE_TEST_DATA_PATH));
        Assert.IsTrue(player.IsSceneCacheOpened());       
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(player, Path.GetFullPath(SPHERE_TEST_DATA_PATH));
        Assert.IsTrue(player.IsSceneCacheOpened());

        //Cleanup
        Object.DestroyImmediate(go);
        
    }

//----------------------------------------------------------------------------------------------------------------------       
    [Test]
    public void ChangeSceneCacheOnDirectPrefab() {
        
        //Initial setup
        const string DEST_PREFAB_PATH = "Assets/TestSceneCache.prefab";
        const string ASSETS_FOLDER    = "Assets/TestSceneCacheAssets";
            
        bool prefabCreated = SceneCachePlayerEditorUtility.CreateSceneCachePlayerAndPrefab(
            Path.GetFullPath(CUBE_TEST_DATA_PATH), DEST_PREFAB_PATH,ASSETS_FOLDER, 
            out SceneCachePlayer player, out GameObject prefab
        );
        Assert.IsTrue(prefabCreated);

        //Check the prefab
        Assert.IsFalse(prefab.IsPrefabInstance());              
        Assert.IsTrue(prefab.IsPrefab());              
        SceneCachePlayer prefabPlayer = prefab.GetComponent<SceneCachePlayer>();
        Assert.IsNotNull(prefabPlayer);
        
        
        //Change
        string newSceneCacheFilePath = Path.GetFullPath(SPHERE_TEST_DATA_PATH);
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(prefabPlayer, newSceneCacheFilePath);
        Assert.AreEqual(newSceneCacheFilePath, prefabPlayer.GetSceneCacheFilePath());
        

        //Cleanup
        Object.DestroyImmediate(player.gameObject);
        DeleteSceneCachePlayerPrefab(prefab);        
    }
    
//----------------------------------------------------------------------------------------------------------------------       
    [Test]
    public void ChangeSceneCacheOnInstancedPrefab() {
        
        //Initial setup
        const string DEST_PREFAB_PATH = "Assets/TestSceneCache.prefab";
        const string ASSETS_FOLDER    = "Assets/TestSceneCacheAssets";
            
        bool prefabCreated = SceneCachePlayerEditorUtility.CreateSceneCachePlayerAndPrefab(
            Path.GetFullPath(CUBE_TEST_DATA_PATH), DEST_PREFAB_PATH,ASSETS_FOLDER, 
            out SceneCachePlayer player, out GameObject prefab
        );
        Assert.IsTrue(prefabCreated);

        //Check the instanced prefab
        Assert.IsNotNull(player);              
        Assert.IsTrue(player.gameObject.IsPrefabInstance());              
        
        
        //Change
        string newSceneCacheFilePath = Path.GetFullPath(SPHERE_TEST_DATA_PATH);
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(player, newSceneCacheFilePath);
        Assert.IsTrue(player.IsSceneCacheOpened());
        Assert.AreEqual(newSceneCacheFilePath, player.GetSceneCacheFilePath());
        

        //Cleanup
        Object.DestroyImmediate(player.gameObject);
        DeleteSceneCachePlayerPrefab(prefab);        
    }
    
//----------------------------------------------------------------------------------------------------------------------       

    private void CreateAndDeleteSceneCachePlayerPrefab(string sceneCachePath) {
        Assert.IsTrue(File.Exists(sceneCachePath));

        const string DEST_PREFAB_PATH  = "Assets/TestSceneCache.prefab";
        const string ASSETS_FOLDER     = "Assets/TestSceneCacheAssets";
            
        bool  prefabCreated  = SceneCachePlayerEditorUtility.CreateSceneCachePlayerAndPrefab(
            Path.GetFullPath(sceneCachePath), DEST_PREFAB_PATH,ASSETS_FOLDER, 
            out SceneCachePlayer player, out GameObject prefab
        );
        Assert.IsTrue(prefabCreated);

        //Check player
        Assert.IsNotNull(player);
        Assert.IsTrue(player.IsSceneCacheOpened());       
        Assert.IsTrue(player.gameObject.IsPrefabInstance());
        
        //Check the prefab
        Assert.IsNotNull(prefab);
        string prefabPath = AssetDatabase.GetAssetPath(prefab);
        Assert.IsFalse(string.IsNullOrEmpty(prefabPath));
        Assert.AreEqual(DEST_PREFAB_PATH, prefabPath);

        
        DeleteSceneCachePlayerPrefab(prefab);
    }
    
//----------------------------------------------------------------------------------------------------------------------

    void DeleteSceneCachePlayerPrefab(GameObject prefab) {
        Assert.IsTrue(prefab.IsPrefab());       
        
        SceneCachePlayer prefabPlayer = prefab.GetComponent<SceneCachePlayer>();
        Assert.IsNotNull(prefabPlayer);

        string assetsFolder = prefabPlayer.GetAssetsFolder();
        
        //Check assets folder
        Assert.IsTrue(Directory.Exists(assetsFolder));
        string[] prefabAssetGUIDs = AssetDatabase.FindAssets("", new[] {assetsFolder});
        Assert.Greater(prefabAssetGUIDs.Length, 0);
        
        
        //Cleanup
        foreach (string guid in prefabAssetGUIDs) {
            AssetDatabase.DeleteAsset(AssetDatabase.GUIDToAssetPath(guid));
        }
        FileUtil.DeleteFileOrDirectory(assetsFolder);

        string prefabPath = AssetDatabase.GetAssetPath(prefab);
        Assert.IsFalse(string.IsNullOrEmpty(prefabPath));
        AssetDatabase.DeleteAsset(prefabPath);

        AssetDatabase.Refresh();                
    }
    

//----------------------------------------------------------------------------------------------------------------------    
    
    private static readonly string TEST_DATA_PATH = Path.Combine("Packages", MeshSyncConstants.PACKAGE_NAME, "Tests", "Data");
    private static readonly string CUBE_TEST_DATA_PATH   = Path.Combine(TEST_DATA_PATH, "Cube.sc");
    private static readonly string SPHERE_TEST_DATA_PATH = Path.Combine(TEST_DATA_PATH, "Sphere.sc");
    
}

}