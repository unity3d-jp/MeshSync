using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal class BlenderIntegrator : BaseDCCIntegrator {
    internal BlenderIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "Blender";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool ConfigureDCCToolV(DCCToolInfo dccToolInfo, string pluginFileNameWithoutExt, 
        string extractedTempPath) 
    {        
        //Go down one folder
        string extractedPath = null;
        foreach (string dir in Directory.EnumerateDirectories(extractedTempPath)) {
            extractedPath = dir;
            break;
        }

        if (string.IsNullOrEmpty(extractedPath))
            return false;

        //Must use '/' for the pluginFile which is going to be inserted into the template
        string ver = dccToolInfo.DCCToolVersion;

        string[] pluginFiles = Directory.GetFiles(extractedPath, $"blender-{ver}*.zip");
        if (pluginFiles.Length <= 0) {
            return false;            
        }
        
        string pluginFile = pluginFiles[0].Replace(Path.DirectorySeparatorChar,'/');        
        if (!File.Exists(pluginFile)) {
            return false;
        }
        
        //Prepare install script
        string installScriptFileName = $"InstallBlenderPlugin_{ver}.py";
        string templatePath = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH,installScriptFileName );
        if (!File.Exists(templatePath)) {
            return false;
        }
        
        //Replace the path in the template with actual path.
        string installScriptFormat = File.ReadAllText(templatePath);
        string installScript = String.Format(installScriptFormat,pluginFile);
        string installScriptPath = Path.Combine(extractedTempPath, installScriptFileName);
        File.WriteAllText(installScriptPath, installScript);
        
        //Prepare remove script to remove old plugin
        string uninstallScriptFilename = $"UninstallBlenderPlugin_{ver}.py";
        string uninstallScriptPath = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH,uninstallScriptFilename );
        if (!File.Exists(uninstallScriptPath)) {
            return false;
        }
      
        bool setupSuccessful = SetupAutoLoadPlugin(dccToolInfo.AppPath, 
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

    
    bool SetupAutoLoadPlugin(string appPath, string uninstallScriptPath, string installScriptPath) {

        try {
            if (!System.IO.File.Exists(appPath)) {
                Debug.LogError("[MeshSync] No Blender installation found at " + appPath);
                return false;
            }

            //Try to uninstall first. The uninstallation may have exceptions/error messages, but they can be ignored
            System.Diagnostics.Process process = new System.Diagnostics.Process {
                StartInfo = {
                    FileName = appPath,
                    // WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden,
                    // CreateNoWindow = true,
                    UseShellExecute = false,
                    RedirectStandardError = true,
                    Arguments = $"-b -P {uninstallScriptPath}"         //Execute batch mode
                },
                EnableRaisingEvents = true
            };            
            process.Start();
            process.WaitForExit();
            
            //Install
            const int PYTHON_EXIT_CODE = 10;
            process.StartInfo.Arguments = $"-b -P {installScriptPath} --python-exit-code {PYTHON_EXIT_CODE}";
            process.Start();
            process.WaitForExit();
            int exitCode = process.ExitCode;
            
            if (0!=exitCode) {
                string stderr = process.StandardError.ReadToEnd();
                Debug.LogError($"[MeshSync] Installation error. ExitCode: {exitCode}. {stderr}");
                return false;
            }
            
            
        } catch (Exception e) {
            Debug.LogError("[MeshSync] Failed to install plugin. Exception: " + e.Message);
            return false;
        }

        return true;
    }
    

    
}

} //end namespace
