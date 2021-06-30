using System;
using System.IO;
using JetBrains.Annotations;
using NUnit.Framework;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal class BlenderIntegrator : BaseDCCIntegrator {
    internal BlenderIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }

//----------------------------------------------------------------------------------------------------------------------
    [CanBeNull]
    internal static string GetInstallScriptTemplatePath(string ver) {
        string installScriptFileName = $"InstallBlenderPlugin_{ver}.py";
        string templatePath = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH,installScriptFileName );
        if (!File.Exists(templatePath)) {
            return null;
        }

        return templatePath;

    }

    [CanBeNull]
    internal static string GetUninstallScriptPath(string ver) {
        string uninstallScriptFilename = $"UninstallBlenderPlugin_{ver}.py";
        string uninstallScriptPath = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH,uninstallScriptFilename );
        if (!File.Exists(uninstallScriptPath)) {
            return null;
        }

        return uninstallScriptPath;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "Blender";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool ConfigureDCCToolV(DCCToolInfo dccToolInfo, string srcPluginRoot, 
        string tempPath) 
    {
        Assert.IsTrue(Directory.Exists(srcPluginRoot));

        //Must use '/' for the pluginFile which is going to be inserted into the template
        string ver = dccToolInfo.DCCToolVersion;

        string   zipFileName = $"blender-{ver}*.zip";
        string[] pluginFiles = Directory.GetFiles(srcPluginRoot, zipFileName);
        if (pluginFiles.Length <= 0) {
            SetLastErrorMessage($"Can't find {zipFileName} in MeshSyncDCCPlugins package");
            return false;            
        }
        
        string pluginFile = pluginFiles[0].Replace(Path.DirectorySeparatorChar,'/');        
        if (!File.Exists(pluginFile)) {
            SetLastErrorMessage($"Can't find plugin file: {pluginFile}");
            return false;
        }
        
        //Prepare install script
        string templatePath = GetInstallScriptTemplatePath(ver);
        if (string.IsNullOrEmpty(templatePath)) {
            SetLastErrorMessage($"Can't find install script template path: {templatePath}");            
            return false;
        }
        
        //Replace the path in the template with actual path.
        string installScriptFormat = File.ReadAllText(templatePath);
        string installScript = String.Format(installScriptFormat,pluginFile);
        string installScriptPath = Path.Combine(tempPath, $"InstallBlenderPlugin_{ver}.py");
        File.WriteAllText(installScriptPath, installScript);
        
        //Prepare remove script to remove old plugin
        string uninstallScriptPath = GetUninstallScriptPath(ver);
        if (string.IsNullOrEmpty(uninstallScriptPath)) {
            SetLastErrorMessage($"Can't find uninstall script path: {uninstallScriptPath}");            
            return false;            
        }
      
        bool setupSuccessful = SetupAutoLoadPlugin(dccToolInfo.AppPath, 
            dccToolInfo.DCCToolVersion,
            Path.GetFullPath(uninstallScriptPath), 
            Path.GetFullPath(installScriptPath)
        );

        //Cleanup
        File.Delete(installScriptPath);

        return setupSuccessful;
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void FinalizeDCCConfigurationV() {
        DCCToolInfo dccToolInfo = GetDCCToolInfo();
        
        EditorUtility.DisplayDialog("MeshSync",
            $"MeshSync plugin configured. Please restart {dccToolInfo.GetDescription()} to complete the installation.", 
            "Ok"
        );
    }

//----------------------------------------------------------------------------------------------------------------------

    
    bool SetupAutoLoadPlugin(string appPath, string dccToolVersion, 
        string uninstallScriptPath, string installScriptPath) {

        try {
            if (!System.IO.File.Exists(appPath)) {
                SetLastErrorMessage("No Blender installation found at " + appPath);
                return false;
            }

            //Try to uninstall first. The uninstallation may have exceptions/error messages, but they can be ignored
            System.Diagnostics.Process process = DiagnosticsUtility.StartProcess(
                appPath, 
                $"-b -P {uninstallScriptPath}",                
                /*useShellExecute=*/ false, /*redirectStandardError=*/ true                 
            );
            process.WaitForExit();
            
            
#if UNITY_EDITOR_OSX
            DeleteInstalledPluginOnMac(dccToolVersion);
#endif            
            
            //Install
            const int PYTHON_EXIT_CODE = 10;
            process.StartInfo.Arguments = $"-b -P \"{installScriptPath}\" --python-exit-code {PYTHON_EXIT_CODE}";
            process.Start();
            process.WaitForExit();
            int exitCode = process.ExitCode;
            
            if (0!=exitCode) {
                string stderr = process.StandardError.ReadToEnd();
                SetLastErrorMessage($"Process error. ExitCode: {exitCode}. {stderr}");
                return false;
            }
                        
        } catch (Exception e) {
            SetLastErrorMessage($"Process error. Exception: {e.Message}");
            return false;
        }

        return true;
    }

//----------------------------------------------------------------------------------------------------------------------        

    void DeleteInstalledPluginOnMac(string blenderVersion) {
        //Delete plugin on mac to avoid errors of loading new plugin:
        //Termination Reason: Namespace CODESIGNING, Code 0x2 
        string installedPluginDir = Environment.GetFolderPath(Environment.SpecialFolder.Personal)
            + $"/Library/Application Support/Blender/{blenderVersion}/scripts/addons/MeshSyncClientBlender";
        if (!Directory.Exists(installedPluginDir)) 
            return;

        string[] files = System.IO.Directory.GetFiles(installedPluginDir, "*.so");            
        if (files.Length <=0)
            return;

        foreach (string binaryPluginFile in files) {
            try {
                File.Delete(binaryPluginFile);
            } catch (Exception e) {
                string errMsg = $"[MeshSync] Error when overwriting plugin: {binaryPluginFile}. Error: {e}";
                SetLastErrorMessage(errMsg);
                Debug.LogError(errMsg);
            }
        }

    }

    
}

} //end namespace
