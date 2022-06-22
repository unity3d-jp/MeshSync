using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;

namespace Unity.MeshSync.Tests {
internal class PluginTests {
    
    //Skip OSX because we don't have a OSXUniversal bundle for mscore 
    [Test]
    [UnityPlatform(RuntimePlatform.WindowsPlayer, RuntimePlatform.LinuxPlayer)]
    public void CheckVersionValidity() {
        string version = Lib.GetPluginVersion();
        Assert.IsTrue(PackageVersion.TryParse(version, out _));
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    
}

} //end namespace
