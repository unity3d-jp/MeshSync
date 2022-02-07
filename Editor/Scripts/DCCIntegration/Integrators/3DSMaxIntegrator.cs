using System;
using System.IO;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal class _3DSMaxIntegrator : BaseDCCIntegrator {
    internal _3DSMaxIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "3DSMAX";
    }

    internal static string GetInstallScriptTemplatePath(string version) {
        string installScriptFileName = $"Install3dsMaxPlugin.ms";
        string templatePath = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH,installScriptFileName );
        if (!File.Exists(templatePath)) {
            return null;
        }

        return templatePath;
    }
    

//----------------------------------------------------------------------------------------------------------------------
    internal override bool ConfigureDCCToolV(DCCToolInfo dccToolInfo, string srcPluginRoot, 
        string tempPath) 
    {        
        Assert.IsTrue(Directory.Exists(srcPluginRoot));
               
        string appVersion = $"3dsMax{dccToolInfo.DCCToolVersion}";
        
        //configFolder example: "C:\Users\Unity\AppData\Local\Unity\MeshSync\3dsMax2019"
        string appDataLocal = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        string configFolder = Path.Combine(appDataLocal, "Unity", "MeshSync", appVersion);        
        Directory.CreateDirectory(configFolder);

        //Copy dlu file to configFolder
        string srcPluginPath = Path.Combine(srcPluginRoot, appVersion);
        if (!Directory.Exists(srcPluginPath)) {
            SetLastErrorMessage($"Can't find src directory: {srcPluginPath}");
            return false;
        }

        try {
            FileUtility.CopyRecursive(srcPluginPath, configFolder, true);
        } catch {
            SetLastErrorMessage($"Failed to copy files to dest: {configFolder}");
            return false;
        }

        bool setupSuccessful = false;

        //Check version
        bool   versionIsInt          = int.TryParse(dccToolInfo.DCCToolVersion, out int versionInt);
        string installScriptPath     = CreateInstallScript(dccToolInfo.DCCToolVersion, configFolder, tempPath);        
        if (versionIsInt && versionInt <= 2018) {
            
            //3dsmax -U MAXScript install_script.ms
            setupSuccessful = SetupAutoLoadPlugin(dccToolInfo.AppPath, $"-U MAXScript \"{installScriptPath}\"");
        } else {
            string dccAppDir = Path.GetDirectoryName(dccToolInfo.AppPath);
            if (string.IsNullOrEmpty(dccAppDir)) {
                SetLastErrorMessage($"Can't determine application directory from: {dccToolInfo.AppPath}");
                return false;                
            }

            //3dsmaxbatch.exe install_script.ms
            string dccBatchPath = Path.Combine(dccAppDir, "3dsmaxbatch.exe");
            setupSuccessful = SetupAutoLoadPlugin(dccBatchPath, installScriptPath);
        }
        
        File.Delete(installScriptPath);
       
       
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
    //create installation script
    private string CreateInstallScript(string dccToolVersion, string configFolder, string tempPath) {
        string templatePath          = GetInstallScriptTemplatePath(dccToolVersion);
        string installScriptFormat   = File.ReadAllText(templatePath);
        string installScript         = String.Format(installScriptFormat,configFolder.Replace("\\","\\\\"));
        string installScriptPath     = Path.Combine(tempPath, $"Install3dsMaxPlugin{dccToolVersion}.ms");
        File.WriteAllText(installScriptPath, installScript);
        return Path.GetFullPath(installScriptPath);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    bool SetupAutoLoadPlugin(string appPath, string startArgument) {
        
        try {
            if (!System.IO.File.Exists(appPath)) {
                SetLastErrorMessage($"No 3dsMax installation found at {appPath}");
                return false;
            }

            EditorUtility.DisplayProgressBar("MeshSync","Launching app to finalize installation",0.75f);
            System.Diagnostics.Process process = DiagnosticsUtility.StartProcess(
                appPath, startArgument            
            );
            process.WaitForExit();
            
            int exitCode = process.ExitCode;
            EditorUtility.ClearProgressBar();               
            
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
    
}

} //end namespace
