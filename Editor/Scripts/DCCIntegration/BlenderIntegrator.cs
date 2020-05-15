using System;
using System.IO;
using Unity.AnimeToolbox;
using Unity.SharpZipLib.Utils;
using UnityEngine;

namespace UnityEditor.MeshSync {
internal class BlenderIntegrator : BaseDCCIntegrator {
    internal BlenderIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }
//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "Blender";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool  ConfigureDCCToolV(DCCToolInfo dccToolInfo, string localPluginPath) 
    {
        string tempPath = FileUtil.GetUniqueTempPathInProject();
        
        Directory.CreateDirectory(tempPath);
        ZipUtility.UncompressFromZip(localPluginPath, null, tempPath);
        
        //Go down one folder
        string extractedPath = null;
        foreach (string dir in Directory.EnumerateDirectories(tempPath)) {
            extractedPath = dir;
            break;
        }

        if (string.IsNullOrEmpty(extractedPath))
            return false;

        //Must use '/' for the pluginFile which is going to be inserted into the template
        string ver = dccToolInfo.DCCToolVersion;
        string pluginFile = Path.Combine(extractedPath, $"blender-{ver}.zip").Replace(Path.DirectorySeparatorChar,'/');
        
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
        string installScriptPath = Path.Combine(tempPath, installScriptFileName);
        File.WriteAllText(installScriptPath, installScript);
        
        //Prepare remove script to remove old plugin
        string uninstallScriptFilename = $"UninstallBlenderPlugin_{ver}.py";
        string uninstallScript = Path.Combine(MeshSyncEditorConstants.DCC_INSTALL_SCRIPTS_PATH,uninstallScriptFilename );
        if (!File.Exists(uninstallScript)) {
            return false;
        }
      
        bool setupSuccessful = SetupAutoLoadPlugin(dccToolInfo.AppPath, uninstallScript, installScriptPath);

        //Cleanup
        File.Delete(installScriptPath);
        FileUtility.DeleteFilesAndFolders(tempPath);

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

    
    bool SetupAutoLoadPlugin(string appPath, string uninstallScriptPath, string installScriptPath) {

        try {
            if (!System.IO.File.Exists(appPath)) {
                Debug.LogError("[MeshSync] No Blender installation found at " + appPath);
                return false;
            }

            //Try to uninstall first. The uninstallation may have error messages, but they can be ignored
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
            process.StartInfo.Arguments = $"-b -P {installScriptPath}";
            process.Start();
            process.WaitForExit();
            string stderr = process.StandardError.ReadToEnd();
            
            if (stderr.Contains("PermissionError")) {
                Debug.LogError($"[MeshSync] Installation error. {stderr}");
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
