using NUnit.Framework;

namespace Unity.MeshSync.Tests {
internal class PluginTests {
    
    [Test]
    public void CheckVersionValidity() {

        string version = Lib.GetPluginVersion();
        Assert.IsTrue(PackageVersion.TryParse(version, out _));

    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    
}

} //end namespace
