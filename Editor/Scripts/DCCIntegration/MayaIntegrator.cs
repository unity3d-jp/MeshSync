﻿using System;
using System.IO;
using Unity.AnimeToolbox;
using Unity.SharpZipLib.Utils;
using UnityEngine;

namespace UnityEditor.MeshSync {

internal class MayaIntegrator : BaseDCCIntegrator {

    internal MayaIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }

//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolName() {
        return "Maya";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override DCCPluginInstallInfo ConfigureDCCTool(DCCToolInfo dccToolInfo, string configFolder, 
        string localPluginPath) 
    {

        string tempPath = FileUtil.GetUniqueTempPathInProject();
        Directory.CreateDirectory(tempPath);
        ZipUtility.UncompressFromZip(localPluginPath, null, tempPath);

        string srcRoot = Path.Combine(tempPath, Path.GetFileNameWithoutExtension(localPluginPath));
        if (!Directory.Exists(srcRoot)) {
            Debug.LogError("[MeshSync] Failed to install DCC Plugin for Maya");
            return null;
        }

        //const string MAYA_CLOSE_COMMAND = "scriptJob -idleEvent quit;";
        //const string AUTOLOAD_SETUP = "pluginInfo -edit -autoload true MeshSyncClientMaya;";
        //const string SHELF_SETUP = "UnityMeshSync_Shelf;";

        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                // If MAYA_APP_DIR environment variable is setup, copy the modules directory there.
                //     If not, go to %USERPROFILE%\Documents\maya in Windows Explorer, and copy the modules directory there.
                //string commandString = "-command \"{0}\"";
                break;
            }
            case RuntimePlatform.OSXEditor: {
                // string commandString = @"-command '{0}'";
                //
                // string loadPlugin =
                //     "loadPlugin \"" + integrationFolder + "/UnityMeshSync/2020/plug-ins/MeshSyncClientMaya.bundle\";";
                //
                // string argument = string.Format(commandString, loadPlugin+AUTOLOAD_SETUP+SHELF_SETUP+MAYA_CLOSE_COMMAND);
                // ConfigureMaya(argument);
                
                //Copy files
                const string MOD_FILE = "UnityMeshSync.mod";
                string SCRIPT_FOLDER = Path.Combine("UnityMeshSync",dccToolInfo.DCCToolVersion);
                File.Copy(Path.Combine(srcRoot,MOD_FILE), Path.Combine(configFolder,MOD_FILE), true);
                FileUtility.CopyRecursive(Path.Combine(srcRoot, SCRIPT_FOLDER), 
                                          Path.Combine(configFolder, SCRIPT_FOLDER),
                                          true);
                
                //Setup Auto Load
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                // Copy the modules directory to ~/maya/<maya_version)            
                break;
            }
            default: {
                throw new NotImplementedException();
            }
        }
        

        //Cleanup
        FileUtility.DeleteFilesAndFolders(tempPath);
        
        return new DCCPluginInstallInfo(dccToolInfo.DCCToolVersion);
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    protected override string FindConfigFolder() {
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                // If MAYA_APP_DIR environment variable is setup, copy the modules directory there.
                //     If not, go to %USERPROFILE%\Documents\maya in Windows Explorer, and copy the modules directory there.
                break;
            }
            case RuntimePlatform.OSXEditor: { return "/Users/Shared/Autodesk/modules/maya"; }
            case RuntimePlatform.LinuxEditor: {
                // Copy the modules directory to ~/maya/<maya_version)            
                break;
            }
            default: {
                throw new NotImplementedException();
            }
        }

        return null;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    // [SecurityPermission(SecurityAction.InheritanceDemand, Flags = SecurityPermissionFlag.UnmanagedCode)]
    // [SecurityPermission(SecurityAction.LinkDemand, Flags = SecurityPermissionFlag.UnmanagedCode)]
    int SetupAutoLoadPlugin(string mayaPath, string startArgument) {
        int exitCode = 0;

        try {
            if (!System.IO.File.Exists(mayaPath)) {
                Debug.LogError("No maya installation found at " + mayaPath);
                return -1;
            }

            System.Diagnostics.Process mayaProcess = new System.Diagnostics.Process();
            mayaProcess.StartInfo.FileName = mayaPath;
            mayaProcess.StartInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            mayaProcess.StartInfo.CreateNoWindow = true;
            mayaProcess.StartInfo.UseShellExecute = false;
            mayaProcess.StartInfo.RedirectStandardError = true;
            mayaProcess.StartInfo.Arguments = startArgument;
            mayaProcess.EnableRaisingEvents = true;
            mayaProcess.Start();

            string stderr = mayaProcess.StandardError.ReadToEnd();
            mayaProcess.WaitForExit();
            exitCode = mayaProcess.ExitCode;
            Debug.Log(string.Format("Ran maya: [{0}]\nWith args [{1}]\nResult {2}",
                mayaPath, mayaProcess.StartInfo.Arguments, exitCode));

            // see if we got any error messages
            if(exitCode != 0 && !string.IsNullOrEmpty(stderr)){
                Debug.LogError(string.Format("Maya installation error (exit code: {0}): {1}", exitCode, stderr));
            }

        } catch (Exception e) {
            UnityEngine.Debug.LogError("Exception failed to start Maya " + e.Message);
            exitCode = -1;
        }
        return exitCode;
    }
    
}


} // end namespace
