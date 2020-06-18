using System;
using System.IO;
using Unity.AnimeToolbox;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal class _3DSMaxIntegrator : BaseDCCIntegrator {
    internal _3DSMaxIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "3DSMAX";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool ConfigureDCCToolV(DCCToolInfo dccToolInfo, string pluginFileNameWithoutExt, 
        string extractedTempPath) 
    {        

        string extractedPluginRootFolder = Path.Combine(extractedTempPath, pluginFileNameWithoutExt);
        if (!Directory.Exists(extractedPluginRootFolder)) {
            return false;
        }
               
        string appVersion = $"3dsMax{dccToolInfo.DCCToolVersion}";
        
        //configFolder example: "C:\Users\Unity\AppData\Local\Unity\MeshSync\3dsMax2019"
        string appDataLocal = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        string configFolder = Path.Combine(appDataLocal, "Unity", "MeshSync", appVersion);        
        Directory.CreateDirectory(configFolder);

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
        string installScriptPath = null;        
        if (versionIsInt && versionInt <= 2018) {
            installScriptPath = CreateInstallScript("Install3dsMaxPlugin2018.ms", configFolder, extractedTempPath);
            
            //3dsmax -U MAXScript install_script.ms
            setupSuccessful = SetupAutoLoadPlugin(dccToolInfo.AppPath, $"-U MAXScript \"{installScriptPath}\"");
        } else {
            installScriptPath = CreateInstallScript("Install3dsMaxPlugin2019.ms", configFolder, extractedTempPath);
            string dccAppDir = Path.GetDirectoryName(dccToolInfo.AppPath);
            if (string.IsNullOrEmpty(dccAppDir))
                return false;

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
    private string CreateInstallScript(string installScriptFileName, string configFolder, string tempPath) {
        string templatePath          = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH, installScriptFileName);
        string installScriptFormat   = File.ReadAllText(templatePath);
        string installScript         = String.Format(installScriptFormat,configFolder.Replace("\\","\\\\"));
        string installScriptPath     = Path.Combine(tempPath, installScriptFileName);
        File.WriteAllText(installScriptPath, installScript);
        return Path.GetFullPath(installScriptPath);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    bool SetupAutoLoadPlugin(string appPath, string startArgument) {
        
        try {
            if (!System.IO.File.Exists(appPath)) {
                Debug.LogError("[MeshSync] No maya installation found at " + appPath);
                return false;
            }

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
            
            EditorUtility.DisplayProgressBar("MeshSync","Launching app to finalize installation",0.75f);
            
            process.Start();
            process.WaitForExit();
            int exitCode = process.ExitCode;
            EditorUtility.ClearProgressBar();               
            
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
