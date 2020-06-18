using System.Collections;
using System.IO;
using NUnit.Framework;
using Unity.MeshSync;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.TestTools;

namespace Unity.MeshSync.Editor.Tests {
internal class MeshSyncServerTest  {
    [UnityTest]
    public IEnumerator CreateAutoServer()  {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);

        //Delete Server Root Path
        string serverRootPath = Path.Combine(Application.streamingAssetsPath,DeployStreamingAssets.SERVER_ROOT_DIR_NAME);
        if (Directory.Exists(serverRootPath))
            Directory.Delete(serverRootPath,true);
        Assert.IsFalse(Directory.Exists(serverRootPath));
        
        MeshSyncServer mss = MeshSyncServerEditor.CreateMeshSyncServer(true);
        Assert.IsTrue(mss.IsServerStarted());
        yield return null;

        //Make sure the server files have been deployed
        Assert.IsTrue(Directory.Exists(serverRootPath));
        
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    [UnityTest]
    public IEnumerator CreateManualServer()  {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);
        MeshSyncServer mss = MeshSyncServerEditor.CreateMeshSyncServer(false);
        Assert.IsFalse(mss.IsServerStarted());

        yield return null;
        Assert.IsFalse(mss.IsServerStarted()); //Still not started

        mss.StartServer();
        yield return null;
        Assert.IsTrue(mss.IsServerStarted());
    }

//----------------------------------------------------------------------------------------------------------------------    
    [UnityTest]
    public IEnumerator CheckServerSettings() {
        MeshSyncServer mss = MeshSyncServerEditor.CreateMeshSyncServer(true);
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();
        Assert.AreEqual(projectSettings.GetDefaultServerPort(), mss.GetServerPort());
        yield return null;
    }


}

} //end namespace
