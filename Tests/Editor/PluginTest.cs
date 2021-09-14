using System.Collections;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine.TestTools;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;

namespace Unity.MeshSync.Editor.Tests {
internal class PluginTest {
    
    [UnityTest]
    public IEnumerator CheckPluginVersion() {

        ListRequest list = Client.List(true, false);
        while (!list.IsCompleted)
            yield return null;

        string pluginVersion = Lib.GetPluginVersion();
        bool   parsed        = PackageVersion.TryParse(pluginVersion, out PackageVersion libVersion);
        Assert.IsTrue(parsed, $"Invalid version: {pluginVersion}");
        
        
        foreach (PackageInfo packageInfo in list.Result) {
            if (packageInfo.name != MeshSyncConstants.PACKAGE_NAME)
                continue;

            parsed = PackageVersion.TryParse(packageInfo.version, out PackageVersion packageVersion);
            Assert.IsTrue(parsed);
            
            
            //Based on our rule to increase the major/minor version whenever we change any plugin code,
            //it's ok for the patch version to be different.
            Assert.AreEqual(libVersion.Major, packageVersion.Major, $"Major: {libVersion.Major} !={packageVersion.Major}");           
            Assert.AreEqual(libVersion.Minor, packageVersion.Minor, $"Minor: {libVersion.Minor} !={packageVersion.Minor}");            
            yield break;
        }
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    /// <summary>
    /// Parse a semantic versioned string to a PackageVersion class
    /// </summary>
    /// <param name="semanticVer">Semantic versioned input string</param>
    /// <param name="packageVersion">The detected PackageVersion. Set to null when the parsing fails</param>
    /// <returns>true if successful, false otherwise</returns>
    public static bool TryParseMajorAndMinorVersion(string semanticVer, out PackageVersion packageVersion) {
        packageVersion = null;
        string[] tokens = semanticVer.Split('.');
        if (tokens.Length <= 2)
            return false;

        if (!int.TryParse(tokens[0], out int major))
            return false;

        if (!int.TryParse(tokens[1], out int minor))
            return false;

        //Find patch and lifecycle
        string[] patches = tokens[2].Split('-');
        if (!int.TryParse(patches[0], out int patch))
            return false;
               
        PackageLifecycle lifecycle = PackageLifecycle.INVALID;
        if (patches.Length > 1) {
            string lifecycleStr = patches[1].ToLower();                    
            switch (lifecycleStr) {
                case "experimental": lifecycle = PackageLifecycle.EXPERIMENTAL; break;
                case "preview"     : lifecycle = PackageLifecycle.PREVIEW; break;
                case "pre"         : lifecycle = PackageLifecycle.PRERELEASE; break;
                default: lifecycle             = PackageLifecycle.INVALID; break;
            }
            
        } else {
            lifecycle = PackageLifecycle.RELEASED; 
            
        }

        packageVersion = new PackageVersion() {
            Major     = major,
            Minor     = minor,
            Patch     = patch,
            Lifecycle = lifecycle
        };

        const int METADATA_INDEX = 3;
        if (tokens.Length > METADATA_INDEX) {
            packageVersion.AdditionalMetadata = String.Join(".",tokens, METADATA_INDEX, tokens.Length-METADATA_INDEX);
        }

        return true;

    } 
    
}

} //end namespace
