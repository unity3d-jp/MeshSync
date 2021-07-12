using System.Collections;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
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

        bool parsed = PackageVersion.TryParse(Lib.GetPluginVersion(), out PackageVersion libVersion);
        Assert.IsTrue(parsed);
        
        
        foreach (PackageInfo packageInfo in list.Result) {
            if (packageInfo.name != MeshSyncConstants.PACKAGE_NAME)
                continue;

            parsed = PackageVersion.TryParse(packageInfo.version, out PackageVersion packageVersion);
            Assert.IsTrue(parsed);
            
            
            //Based on our rule to increase the major/minor version whenever we change any plugin code,
            //it's ok for the patch version to be different.
            Assert.AreEqual(libVersion.Major, packageVersion.Major);           
            Assert.AreEqual(libVersion.Minor, packageVersion.Minor);            
            yield break;
        }
        
    }
}

} //end namespace
