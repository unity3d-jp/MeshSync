using System.Collections;
using NUnit.Framework;
using Unity.MeshSync;
using UnityEngine.TestTools;
using UnityEditor.PackageManager;

namespace UnityEditor.MeshSync.Tests {
public class PluginTest {
    
    [UnityTest]
    public IEnumerator CheckPluginVersion() {

        PackageManager.Requests.ListRequest list = Client.List(true, false);
        while (!list.IsCompleted)
            yield return null;

        foreach (PackageManager.PackageInfo packageInfo in list.Result) {
            if (packageInfo.name != "com.unity.meshsync")
                continue;
            
            
            Assert.AreEqual(Lib.GetPluginVersion(),packageInfo.version);
            yield break;
        }
        
    }
}
} //end namespace
