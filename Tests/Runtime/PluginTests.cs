using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;

namespace Unity.MeshSync.Tests {
internal class PluginTests {
    
    [Test]
    public void CheckVersion() {

        string version = Lib.GetPluginVersion();
        Assert.IsFalse(string.IsNullOrEmpty(version));
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    
}

} //end namespace
