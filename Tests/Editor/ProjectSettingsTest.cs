using System.Collections.Generic;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;

namespace UnityEditor.MeshSync.Tests {
internal class ProjectSettingsTest {
    
    
    [Test]
    public void CheckInstalledDCCTools() {

        Dictionary<string, DCCToolInfo> dccToolInfos = DCCFinderUtility.FindInstalledDCCTools();
        Assert.GreaterOrEqual(dccToolInfos.Count,0);
    }    
    
//----------------------------------------------------------------------------------------------------------------------    
    [Test]
    public void CheckCurrentSettings() {
        MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
        Assert.NotNull(settings);
    }    

//----------------------------------------------------------------------------------------------------------------------    
    
    [Test]
    [UnityPlatform(RuntimePlatform.OSXEditor)]
    public void FindDCCToolVersionsOnOSX() {

        //[TODO-sin: 2020-5-7] Add tests for Windows and Linux
        string version = null;
        version = DCCFinderUtility.FindMayaVersion("/Applications/Maya 2019/Maya.app/Contents/MacOS/Maya");
        Assert.AreEqual("2019", version);

        version = DCCFinderUtility.FindMayaVersion("/Applications/Maya2019/Maya.app/Contents/MacOS/Maya");
        Assert.AreEqual("2019", version);

    }    
    
}

} //end namespace
