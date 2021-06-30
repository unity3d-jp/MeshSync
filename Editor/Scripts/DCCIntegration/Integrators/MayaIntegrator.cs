using System;
using System.IO;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using Unity.SharpZipLib.Utils;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

internal class MayaIntegrator : BaseDCCIntegrator {
    
    internal MayaIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }

//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileNameV() {
        return "Maya";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool ConfigureDCCToolV(DCCToolInfo dccToolInfo, string srcPluginRoot, 
        string tempPath) 
    {
        Assert.IsTrue(Directory.Exists(srcPluginRoot));

        string configFolder = FindConfigFolder();
        
        const string AUTOLOAD_SETUP = "pluginInfo -edit -autoload true MeshSyncClientMaya;";
        const string SHELF_SETUP = "UnityMeshSync_Shelf;";
        //const string MAYA_CLOSE_COMMAND = "scriptJob -idleEvent quit;";
        const string FINALIZE_SETUP = AUTOLOAD_SETUP + SHELF_SETUP;
        
        string copySrcFolder  = srcPluginRoot;
        string copyDestFolder = configFolder;
        string argFormat      = null;
        string loadPluginCmd  = null;
            
            
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                //C:\Users\Unity\Documents\maya\modules
                const string FOLDER_PREFIX = "modules";
                copySrcFolder  = Path.Combine(srcPluginRoot, FOLDER_PREFIX);
                copyDestFolder = Path.Combine(configFolder, FOLDER_PREFIX);                
                
                argFormat = "-command \"{0}\"";

                //Maya script only supports '/' as PathSeparator
                //Example: loadPlugin """C:/Users/Unity/Documents/maya/modules/UnityMeshSync/2019/plug-ins/MeshSyncClientMaya.mll""";
                string mayaPluginPath = Path.Combine(copyDestFolder, "UnityMeshSync", dccToolInfo.DCCToolVersion, 
                    @"plug-ins\MeshSyncClientMaya.mll").Replace('\\','/');
                loadPluginCmd = "loadPlugin \"\"\"" + mayaPluginPath + "\"\"\";";
                break;
            }
            case RuntimePlatform.OSXEditor: {
                
                argFormat = @"-command '{0}'";
                //Example: "/Users/Shared/Autodesk/Modules/maya/UnityMeshSync/2020/plug-ins/MeshSyncClientMaya.bundle";
                string mayaPluginPath = Path.Combine(copyDestFolder, "UnityMeshSync", dccToolInfo.DCCToolVersion, 
                    @"plug-ins/MeshSyncClientMaya.bundle");
                
                loadPluginCmd = "loadPlugin \"" + mayaPluginPath + "\";";
                
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                //Example: /home/Unity/maya/2019/modules
                const string FOLDER_PREFIX = "modules";
                copySrcFolder  = Path.Combine(srcPluginRoot, FOLDER_PREFIX);
                copyDestFolder = Path.Combine(configFolder, FOLDER_PREFIX);

                argFormat = @"-command '{0}'";

                string mayaPluginPath = Path.Combine(copyDestFolder, "UnityMeshSync", dccToolInfo.DCCToolVersion, 
                    @"plug-ins/MeshSyncClientMaya.so");
                loadPluginCmd = "loadPlugin \"" + mayaPluginPath + "\";";
                
                break;
            }
            default: {
                throw new NotImplementedException();
            }
        }

        //Copy files to config folder
        const string MOD_FILE = "UnityMeshSync.mod";
        string scriptFolder = Path.Combine("UnityMeshSync",dccToolInfo.DCCToolVersion);
        string srcModFile = Path.Combine(copySrcFolder, MOD_FILE);
        if (!File.Exists(srcModFile)) {
            SetLastErrorMessage($"Can't find mod file: {srcModFile}");
            return false;
        }
        try {
            Directory.CreateDirectory(copyDestFolder);
            File.Copy(srcModFile, Path.Combine(copyDestFolder, MOD_FILE), true);
            FileUtility.CopyRecursive(Path.Combine(copySrcFolder, scriptFolder),
                Path.Combine(copyDestFolder, scriptFolder),
                true);
        } catch {
            SetLastErrorMessage($"Failed to copy files to dest: {copyDestFolder}");
            return false;
        }


        //Auto Load
        string arg = string.Format(argFormat, loadPluginCmd+FINALIZE_SETUP);
        bool setupSuccessful = SetupAutoLoadPlugin(dccToolInfo.AppPath, arg);

        return setupSuccessful;
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    protected override void FinalizeDCCConfigurationV() {
        DCCToolInfo dccToolInfo = GetDCCToolInfo();
        
        EditorUtility.DisplayDialog("MeshSync",
            $"Launching {dccToolInfo.GetDescription()} for finalizing configuration", 
            "Ok"
        );                
        
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    private string FindConfigFolder() {
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {

                //If MAYA_APP_DIR environment variable is setup, use that config folder
                //If not, use %USERPROFILE%\Documents\maya 
                string path = Environment.GetEnvironmentVariable("MAYA_APP_DIR");
                if (!string.IsNullOrEmpty(path))
                    return path;

                path = Directory.GetParent(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)).FullName;
                if (Environment.OSVersion.Version.Major >= 6) {
                    path = Directory.GetParent(path).ToString();
                }
                path+= @"\Documents\maya";
                return path;
            }

            case RuntimePlatform.OSXEditor: { return "/Users/Shared/Autodesk/modules/maya"; }
            case RuntimePlatform.LinuxEditor: {
                string userProfile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                DCCToolInfo dccToolInfo = GetDCCToolInfo();
                return Path.Combine(userProfile, "maya", dccToolInfo.DCCToolVersion);
            }
            default: {
                throw new NotImplementedException();
            }
        }

    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    // [SecurityPermission(SecurityAction.InheritanceDemand, Flags = SecurityPermissionFlag.UnmanagedCode)]
    // [SecurityPermission(SecurityAction.LinkDemand, Flags = SecurityPermissionFlag.UnmanagedCode)]
    bool SetupAutoLoadPlugin(string mayaPath, string startArgument) {
        
        try {
            if (!System.IO.File.Exists(mayaPath)) {
                SetLastErrorMessage($"No Maya installation found at {mayaPath}");
                return false;
            }

            //[note-sin: 2020-5-12] WindowStyle=Hidden (requires UseShellExecute=true and RedirectStandardError=false),
            //seems to be able to hide only the splash screen, but not the Maya window.
            System.Diagnostics.Process mayaProcess = new System.Diagnostics.Process {
                StartInfo = {
                    FileName = mayaPath,
                    // WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden,
                    // CreateNoWindow = true,
                    UseShellExecute = true,
                    RedirectStandardError = false,
                    Arguments = startArgument
                },
                EnableRaisingEvents = true
            };
            mayaProcess.Start();

            // string stderr = mayaProcess.StandardError.ReadToEnd();
            // mayaProcess.WaitForExit();
            // int exitCode = mayaProcess.ExitCode;
            
        } catch (Exception e) {
            SetLastErrorMessage($"Process error. Exception: {e.Message}");
            return false;
        }

        return true;
    }
    
    
}


} // end namespace
