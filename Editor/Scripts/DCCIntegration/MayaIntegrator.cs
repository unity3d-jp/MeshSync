using System;
using System.IO;
using Unity.AnimeToolbox;
using Unity.SharpZipLib.Utils;
using UnityEngine;

namespace UnityEditor.MeshSync {

internal class MayaIntegrator : BaseDCCIntegrator {
    
    internal MayaIntegrator(DCCToolInfo dccToolInfo) : base(dccToolInfo) { }

//----------------------------------------------------------------------------------------------------------------------

    protected override string GetDCCToolInFileName() {
        return "Maya";
    }

//----------------------------------------------------------------------------------------------------------------------
    protected override bool ConfigureDCCTool(DCCToolInfo dccToolInfo, string configFolder, string localPluginPath) 
    {
        string tempPath = FileUtil.GetUniqueTempPathInProject();
        
        Directory.CreateDirectory(tempPath);
        ZipUtility.UncompressFromZip(localPluginPath, null, tempPath);

        string srcRoot = Path.Combine(tempPath, Path.GetFileNameWithoutExtension(localPluginPath));
        if (!Directory.Exists(srcRoot)) {
            return false;
        }

        const string AUTOLOAD_SETUP = "pluginInfo -edit -autoload true MeshSyncClientMaya;";
        const string SHELF_SETUP = "UnityMeshSync_Shelf;";
        const string MAYA_CLOSE_COMMAND = "scriptJob -idleEvent quit;";
        const string FINALIZE_SETUP = AUTOLOAD_SETUP + SHELF_SETUP + MAYA_CLOSE_COMMAND;
        
        string copySrcFolder  = srcRoot;
        string copyDestFolder = configFolder;
        string argFormat = null;
        string loadPluginCmd = null;
            
            
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                //C:\Users\Unity\Documents\maya\modules
                const string FOLDER_PREFIX = "modules";
                copySrcFolder  = Path.Combine(srcRoot, FOLDER_PREFIX);
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
                loadPluginCmd = "loadPlugin \"" + configFolder + "/UnityMeshSync/"+ dccToolInfo.DCCToolVersion 
                                       + "/plug-ins/MeshSyncClientMaya.bundle\";";
                
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                //Example: /home/Unity/maya/2019/modules
                const string FOLDER_PREFIX = "modules";
                copySrcFolder  = Path.Combine(srcRoot, FOLDER_PREFIX);
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
            return false;
        }
        try {
            Directory.CreateDirectory(copyDestFolder);
            File.Copy(srcModFile, Path.Combine(copyDestFolder, MOD_FILE), true);
            FileUtility.CopyRecursive(Path.Combine(copySrcFolder, scriptFolder),
                Path.Combine(copyDestFolder, scriptFolder),
                true);
        } catch {
            return false;
        }


        //Auto Load
        string arg = string.Format(argFormat, loadPluginCmd+FINALIZE_SETUP);
        int exitCode = SetupAutoLoadPlugin(dccToolInfo.AppPath, arg);

        //Cleanup
        FileUtility.DeleteFilesAndFolders(tempPath);

        return (0 == exitCode);
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    protected override string FindConfigFolder() {
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
    int SetupAutoLoadPlugin(string mayaPath, string startArgument) {
        int exitCode = 0;
        
        try {
            if (!System.IO.File.Exists(mayaPath)) {
                Debug.LogError("[MeshSync] No maya installation found at " + mayaPath);
                return -1;
            }

            //[note-sin: 2020-5-12] WindowStyle=Hidden (requires UseShellExecute=true and RedirectStandardError=false),
            //seems to be able to hide only the splash screen, but not the Maya window.
            System.Diagnostics.Process mayaProcess = new System.Diagnostics.Process {
                StartInfo = {
                    FileName = mayaPath,
//                    WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden,
                    CreateNoWindow = true,
                    UseShellExecute = false,
                    RedirectStandardError = true,
                    Arguments = startArgument
                },
                EnableRaisingEvents = true
            };
            mayaProcess.Start();

            string stderr = mayaProcess.StandardError.ReadToEnd();
            mayaProcess.WaitForExit();
            exitCode = mayaProcess.ExitCode;
            // Debug.Log(string.Format("Ran maya: [{0}]\nWith args [{1}]\nResult {2}",
            //     mayaPath, mayaProcess.StartInfo.Arguments, exitCode));

            // see if we got any error messages
            if(exitCode != 0 && !string.IsNullOrEmpty(stderr)){
                Debug.LogError($"[MeshSync] Maya installation error (exit code: {exitCode}): {stderr}");
            }

        } catch (Exception e) {
            Debug.LogError("[MeshSync] Failed to start Maya. Exception: " + e.Message);
            exitCode = -1;
        }
        return exitCode;
    }
    
    
}


} // end namespace
