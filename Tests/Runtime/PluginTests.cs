using System.Collections;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;

//Skip OSXPlayer because we don't have a OSXUniversal bundle for mscore 
namespace Unity.MeshSync.Tests {
internal class PluginTests {
    
    [Test]
    [UnityPlatform(RuntimePlatform.WindowsPlayer, RuntimePlatform.WindowsEditor, RuntimePlatform.LinuxPlayer, 
        RuntimePlatform.LinuxEditor, RuntimePlatform.OSXEditor)]
    public void CheckVersionValidity() {
        string version = Lib.GetPluginVersion();
        Assert.IsTrue(PackageVersion.TryParse(version, out _));
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    [UnityTest]
    [UnityPlatform(RuntimePlatform.WindowsPlayer, RuntimePlatform.WindowsEditor, RuntimePlatform.LinuxPlayer, 
        RuntimePlatform.LinuxEditor, RuntimePlatform.OSXEditor)]
    public IEnumerator CreateServerGameObject() {
        MeshSyncServer goServer  = new GameObject("Server").AddComponent<MeshSyncServer>();
        goServer.SetAutoStartServer(true);
        yield return null;

        Assert.IsTrue(Server.IsStarted(goServer.GetServerPort()));
        yield return null;
    }
    
    
    
    
}

} //end namespace
