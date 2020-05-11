using System.Collections.Generic;
using System.IO;
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
    [UnityPlatform(RuntimePlatform.OSXEditor,RuntimePlatform.LinuxEditor)]
    public void FindDCCToolVersionsOnOSXAndLinux() {
        string version = null;
        version = DCCFinderUtility.FindMayaVersion("/Applications/Maya 2019/Maya.app/Contents/MacOS/Maya");
        Assert.AreEqual("2019", version);

        version = DCCFinderUtility.FindMayaVersion("/Applications/Maya2019/Maya.app/Contents/MacOS/Maya");
        Assert.AreEqual("2019", version);
    }    

    [Test]
    [UnityPlatform(RuntimePlatform.WindowsEditor)]
    public void FindDCCToolVersionsOnWindows() {

        string version = null;
        version = DCCFinderUtility.FindMayaVersion(@"C:\Program Files\Autodesk\maya2019\bin\maya.exe");
        Assert.AreEqual("2019", version);

        version = DCCFinderUtility.FindMayaVersion(@"C:\Program Files\Autodesk\maya2020\bin\maya.exe");
        Assert.AreEqual("2020", version);

        version = DCCFinderUtility.Find3DSMaxVersion(@"C:\Program Files\Autodesk\3ds Max 2019\3dsmax.exe");
        Assert.AreEqual("2019", version);

        version = DCCFinderUtility.Find3DSMaxVersion(@"C:\Program Files\Autodesk\3ds Max 2020\3dsmax.exe");
        Assert.AreEqual("2020", version);

    }    
    
}

} //end namespace
