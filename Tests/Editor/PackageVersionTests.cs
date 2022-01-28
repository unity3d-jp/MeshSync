using System.Collections.Generic;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;

namespace Unity.MeshSync.Editor.Tests {
internal class PackageVersionTests {
    [Test]
    public void CheckPackageCompatibility() {

        bool isParsed = PackageVersion.TryParse("0.1.2-preview", out PackageVersion packageVersion);
        Assert.IsTrue(isParsed);

        List<string> compatiblePackages = new List<string>() {
            "0.1.3-preview",
            "0.1.4",
            "0.1.0-preview",
            "0.1.1-preview",
            "0.1.2",
        };

        foreach (string str in compatiblePackages) {
            Assert.IsTrue(DCCToolsSettingsTab.IsPackageVersionCompatible(str, packageVersion, out _));            
        }

        List<string> inCompatiblePackages = new List<string>() {
            null, 
            "",
            "...",
            "1.1.2-preview",
            "0.2.0-preview",
            "1.2.0",
        };
        
        foreach (string str in inCompatiblePackages) {
            Assert.IsFalse(DCCToolsSettingsTab.IsPackageVersionCompatible(str, packageVersion, out _));            
        }

    }
}

} //end namespace
