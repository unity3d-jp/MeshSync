using System;
using System.IO;
using Unity.AnimeToolbox;
using UnityEngine;

namespace UnityEditor.MeshSync {
internal class _3DSMaxIntegrator : BaseDCCIntegrator {
    internal _3DSMaxIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "3DSMAX";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool ConfigureDCCToolV(DCCToolInfo dccToolInfo, string pluginFileName, string extractedTempPath) 
    {        

        string appVersion = $"3dsMax{dccToolInfo.DCCToolVersion}";
        string configFolder = FindConfigFolder(appVersion);
        Directory.CreateDirectory(configFolder);
        
        //Go down one folder
        string extractedPluginRootFolder = null;
        foreach (string dir in Directory.EnumerateDirectories(extractedTempPath)) {
            extractedPluginRootFolder = dir;
            break;
        }

        if (null == extractedPluginRootFolder)
            return false;
        
        //Copy dlu file to configFolder
        string srcPluginPath = Path.Combine(extractedPluginRootFolder, appVersion);
        if (!Directory.Exists(srcPluginPath)) {
            return false;
        }

        try {
            FileUtility.CopyRecursive(srcPluginPath, configFolder, true);
        } catch {
            return false;
        }

        bool setupSuccessful = false;

        //Check version
        bool versionIsInt = int.TryParse(dccToolInfo.DCCToolVersion, out int versionInt);
        if (versionIsInt && versionInt <= 2018) {
            string installScriptPath = CreateInstallScript("Install3dsMaxPlugin2018.ms", configFolder, extractedTempPath);
            
            //3dsmax -U MAXScript install_script.ms
            setupSuccessful = SetupAutoLoadPlugin(dccToolInfo.AppPath, $"-U MAXScript {installScriptPath}");
        } else {
            string installScriptPath = CreateInstallScript("Install3dsMaxPlugin2019.ms", configFolder, extractedTempPath);
            string dccAppDir = Path.GetDirectoryName(dccToolInfo.AppPath);
            if (string.IsNullOrEmpty(dccAppDir))
                return false;

            //3dsmaxbatch.exe install_script.ms
            string dccBatchPath = Path.Combine(dccAppDir, "3dsmaxbatch.exe");
            setupSuccessful = SetupAutoLoadPlugin(dccBatchPath, installScriptPath);
        }
        
       
        return setupSuccessful;
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override void FinalizeDCCConfigurationV() {
        DCCToolInfo dccToolInfo = GetDCCToolInfo();
        
        EditorUtility.DisplayDialog("MeshSync",
            $"MeshSync plugin installed for {dccToolInfo.GetDescription()}", 
            "Ok"
        );
    }
    
//----------------------------------------------------------------------------------------------------------------------
    private string FindConfigFolder(string appVersion) {

        //Sample: "C:\Users\Unity\AppData\Local\Unity\MeshSync\3dsMax2019"
        string appDataLocal = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        return Path.Combine(appDataLocal, "Unity", "MeshSync", appVersion);
        
    }

//----------------------------------------------------------------------------------------------------------------------
    //create installation script
    private string CreateInstallScript(string installScriptFileName, string configFolder, string tempPath) {
        string templatePath          = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH, installScriptFileName);
        string installScriptFormat   = File.ReadAllText(templatePath);
        string installScript         = String.Format(installScriptFormat,configFolder.Replace("\\","\\\\"));
        string installScriptPath     = Path.Combine(tempPath, installScriptFileName);
        File.WriteAllText(installScriptPath, installScript);
        return installScriptPath;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    bool SetupAutoLoadPlugin(string appPath, string startArgument) {
        
        try {
            if (!System.IO.File.Exists(appPath)) {
                Debug.LogError("[MeshSync] No maya installation found at " + appPath);
                return false;
            }

            //[note-sin: 2020-5-12] WindowStyle=Hidden (requires UseShellExecute=true and RedirectStandardError=false),
            //seems to be able to hide only the splash screen, but not the Maya window.
            System.Diagnostics.Process process = new System.Diagnostics.Process {
                StartInfo = {
                    FileName = appPath,
                    // WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden,
                    // CreateNoWindow = true,
                    UseShellExecute       = true,
                    RedirectStandardError = false,
                    Arguments             = startArgument
                },
                EnableRaisingEvents = true
            };
            process.Start();
            process.WaitForExit();
            int exitCode = process.ExitCode;

            if (0!=exitCode) {
                string stderr = process.StandardError.ReadToEnd();
                Debug.LogError($"[MeshSync] 3dsMax plugin installation error. ExitCode: {exitCode}. {stderr}");
                return false;
            }
            
        } catch (Exception e) {
            Debug.LogError("[MeshSync] Failed to setup 3dsMax plugin. Exception: " + e.Message);
            return false;
        }

        return true;
    }
    
}

} //end namespace
