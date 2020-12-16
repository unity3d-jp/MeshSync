using System.Collections;
using System.Collections.Generic;
using System.IO;
using NUnit.Framework;
using Unity.MeshSync;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.TestTools;

namespace Unity.MeshSync.Editor.Tests {
internal class DCCInstallScriptsTest {
    [Test]
    public void CheckSupportedBlenderInstallScripts() {
        //Create a list containing unique versions of Blender
        HashSet<string> versions = new HashSet<string>();
        foreach (var dccToolInfo in MeshSyncEditorConstants.DEFAULT_DCC_TOOLS_BY_FOLDER) {
            if (dccToolInfo.Value.Type != DCCToolType.BLENDER)
                continue;

            string curVer = dccToolInfo.Value.DCCToolVersion;
            if (versions.Contains(curVer) || string.IsNullOrEmpty(curVer))
                continue;
            
            versions.Add(curVer);
        }

        foreach (string ver in versions) {
            string installScriptTemplatePath = BlenderIntegrator.GetInstallScriptTemplatePath(ver);
            Assert.IsFalse(string.IsNullOrEmpty(installScriptTemplatePath), 
                $"Install script Template for Blender {ver} is missing");

            string uninstallScriptPath = BlenderIntegrator.GetUninstallScriptPath(ver);
            Assert.IsFalse(string.IsNullOrEmpty(uninstallScriptPath),             
                $"Uninstall script for Blender {ver} is missing");

        }

    }

}
//----------------------------------------------------------------------------------------------------------------------    

} //end namespace
