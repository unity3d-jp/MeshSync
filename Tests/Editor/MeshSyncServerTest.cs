using System.Collections;
using NUnit.Framework;
using Unity.MeshSync;
using UnityEditor.SceneManagement;
using UnityEngine.TestTools;

namespace UnityEditor.MeshSync.Tests {
internal class MeshSyncServerTest  {
    [UnityTest]
    public IEnumerator CreateAutoServer()  {
        EditorSceneManager.NewScene(NewSceneSetup.DefaultGameObjects);
        MeshSyncServer mss = MeshSyncServerEditor.CreateMeshSyncServer(true);
        Assert.IsTrue(mss.IsServerStarted());


        
        //[TODO-sin: 2020-2-26] Add test to make sure that the StreamingAssets are populated
        Assert.True(true);
        
        yield return null;
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

}

} //end namespace
