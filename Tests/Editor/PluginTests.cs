using System.Collections;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine.TestTools;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;

namespace Unity.MeshSync.Editor.Tests {
internal class PluginTests {
    
    [UnityTest]
    public IEnumerator CheckPluginVersionCompatibility() {

        ListRequest list = Client.List(true, false);
        while (!list.IsCompleted)
            yield return null;

        PackageVersion pluginVersion = MeshSyncEditorConstants.GetPluginVersion();

        int? pluginMajor = pluginVersion.GetMajor();
        int? pluginMinor = pluginVersion.GetMinor();
        int? pluginPatch = pluginVersion.GetPatch();
        
        Assert.IsNotNull(pluginMajor);
        Assert.IsNotNull(pluginMinor);
        
        bool pluginVersionValid = !(pluginMajor == 0 && pluginMinor == 0 && pluginPatch == 0); 
        Assert.IsTrue(pluginVersionValid, $"Plugin version is not valid: {pluginVersion}" );
        
        foreach (PackageInfo packageInfo in list.Result) {
            if (packageInfo.name != MeshSyncConstants.PACKAGE_NAME)
                continue;

            bool parsed = PackageVersion.TryParse(packageInfo.version, out PackageVersion packageVersion);
            Assert.IsTrue(parsed);
                        
            //Based on our rule to increase the major/minor version whenever we change any plugin code,
            //it's ok for the patch version to be different.
            Assert.AreEqual(pluginMajor, packageVersion.GetMajor(), $"Major: {pluginMajor} !={packageVersion.GetMajor()}");           
            Assert.AreEqual(pluginMinor, packageVersion.GetMinor(), $"Minor: {pluginMinor} !={packageVersion.GetMinor()}");            
            yield break;
        }
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    
}

} //end namespace
