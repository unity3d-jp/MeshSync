using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;

namespace Unity.MeshSync.Editor.Tests {
public class PluginTest {
    
    [UnityTest]
    public IEnumerator CheckPluginVersion() {

        ListRequest list = Client.List(true, false);
        while (!list.IsCompleted)
            yield return null;

        foreach (PackageInfo packageInfo in list.Result) {
            if (packageInfo.name != MeshSyncConstants.PACKAGE_NAME)
                continue;
            
            
            Assert.AreEqual(Lib.GetPluginVersion(),packageInfo.version);
            yield break;
        }
        
    }
}

} //end namespace
