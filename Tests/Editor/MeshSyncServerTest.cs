using NUnit.Framework;

namespace UnityEditor.MeshSync.Tests {
public class MeshSyncServerTest  {
    [Test]
    public void CreateServer()
    {
        MeshSyncServerEditor.CreateMeshSyncServer();
        
        //[TODO-sin: 2020-2-26] Add test to make sure that the StreamingAssets are populated
        Assert.True(true);
        
    }

}

} //end namespace
