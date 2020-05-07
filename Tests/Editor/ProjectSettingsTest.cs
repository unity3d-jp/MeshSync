using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;
using UnityScript.Macros;

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
    
    [UnityTest]
    public IEnumerator DownloadDCCPlugin()  {
        string destFolder = FileUtil.GetUniqueTempPathInProject();


        string[] requestedDCCPlatformNames = new string[] {
            "Maya_Windows.zip",
        };

        DCCPluginDownloader downloader = new DCCPluginDownloader(false, destFolder, requestedDCCPlatformNames);

        bool finished = false;
        downloader.Execute((List<string> dccPluginLocalPaths) => {
        
            Assert.AreEqual(requestedDCCPlatformNames.Length, dccPluginLocalPaths.Count);

            //Clean up files
            foreach (string tempDCCPluginLocalPath in dccPluginLocalPaths) {
                Assert.True(File.Exists(tempDCCPluginLocalPath));
                File.Delete(tempDCCPluginLocalPath);
            }

            finished = true;
        }, () =>
        {
            finished = true;
            Assert.Fail();
            
        });

        while (!finished) {
            Thread.Sleep(1000);
            Debug.Log("Downloading ... ");
            yield return null;
        }
        
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
