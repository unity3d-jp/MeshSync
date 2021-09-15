using System.Collections;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine.TestTools;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;
using UnityEngine;

namespace Unity.MeshSync.Editor.Tests {
internal class PluginTest {
    
    [UnityTest]
    public IEnumerator CheckPluginVersion() {

        ListRequest list = Client.List(true, false);
        while (!list.IsCompleted)
            yield return null;

        PackageVersion pluginVersion = MeshSyncEditorConstants.PACKAGE_VERSION;
        Assert.IsFalse(pluginVersion.Major == 0 && pluginVersion.Minor == 0 && pluginVersion.Patch == 0);
        
        foreach (PackageInfo packageInfo in list.Result) {
            if (packageInfo.name != MeshSyncConstants.PACKAGE_NAME)
                continue;

            bool parsed = PackageVersion.TryParse(packageInfo.version, out PackageVersion packageVersion);
            Assert.IsTrue(parsed);
                        
            //Based on our rule to increase the major/minor version whenever we change any plugin code,
            //it's ok for the patch version to be different.
            Assert.AreEqual(pluginVersion.Major, packageVersion.Major, $"Major: {pluginVersion.Major} !={packageVersion.Major}");           
            Assert.AreEqual(pluginVersion.Minor, packageVersion.Minor, $"Minor: {pluginVersion.Minor} !={packageVersion.Minor}");            
            yield break;
        }
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    //Parse only the major and minor version of semantic versioning
    private static bool TryParseMajorAndMinorVersion(string semanticVer, out Vector2Int version) {
        version = Vector2Int.zero;
        string[] tokens = semanticVer.Split('.');
        if (tokens.Length <= 2)
            return false;

        if (!int.TryParse(tokens[0], out int major))
            return false;

        if (!int.TryParse(tokens[1], out int minor))
            return false;

        version = new Vector2Int(major, minor);
        return true;
    } 
    
}

} //end namespace
