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
        HashSet<string> versions = FindUniqueVersionsOfSupportedDCCTools(DCCToolType.BLENDER);
        
        foreach (string ver in versions) {
            string installScriptTemplatePath = BlenderIntegrator.GetInstallScriptTemplatePath(ver);
            Assert.IsFalse(string.IsNullOrEmpty(installScriptTemplatePath), 
                $"Install script Template for Blender {ver} is missing");

            string uninstallScriptPath = BlenderIntegrator.GetUninstallScriptPath(ver);
            Assert.IsFalse(string.IsNullOrEmpty(uninstallScriptPath),             
                $"Uninstall script for Blender {ver} is missing");

        }

    }

//----------------------------------------------------------------------------------------------------------------------
    
    [Test]
    public void CheckSupported3DSMaxInstallScripts() {
        HashSet<string> versions = FindUniqueVersionsOfSupportedDCCTools(DCCToolType.AUTODESK_3DSMAX);
        
        foreach (string ver in versions) {
            string installScriptTemplatePath = _3DSMaxIntegrator.GetInstallScriptTemplatePath(ver);
            Assert.IsFalse(string.IsNullOrEmpty(installScriptTemplatePath), 
                $"Install script Template for 3DSMax {ver} is missing");
        }
    }

//----------------------------------------------------------------------------------------------------------------------
    
    HashSet<string> FindUniqueVersionsOfSupportedDCCTools(DCCToolType dccToolType) {
        HashSet<string> versions = new HashSet<string>();
        foreach (KeyValuePair<string, DCCToolInfo> dccToolInfo in MeshSyncEditorConstants.SUPPORTED_DCC_TOOLS_BY_FOLDER) {
            if (dccToolInfo.Value.Type != dccToolType)
                continue;

            string curVer = dccToolInfo.Value.DCCToolVersion;
            if (versions.Contains(curVer) || string.IsNullOrEmpty(curVer))
                continue;
            
            versions.Add(curVer);
        }

        return versions;
    }
    
}
//----------------------------------------------------------------------------------------------------------------------    

} //end namespace
