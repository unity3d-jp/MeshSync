﻿using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;


namespace UnityEditor.MeshSync {

public class ProjectSettingsUtility {
    
    //Find the location of supported DCC tools on Windows and Mac.
    //Currently only support Autodesk products
    private static List<string> GetDefaultVendorDirectories() {

        List<string> existingDirectories = new List<string>();
        List<string> searchDirectories = new List<string>();

        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                //[TODO-sin: 2020-4-27]: Test on Windows
                DriveInfo[] allDrives = DriveInfo.GetDrives();
                foreach (DriveInfo drive in allDrives) {
                    searchDirectories.Add(Path.Combine(drive.Name, "Program Files/Autodesk") );
                }
                break;
            }
            case RuntimePlatform.OSXEditor: {
                searchDirectories.Add("/Applications/Autodesk");
                break;
            }
            default: {
                throw new System.NotImplementedException();
            }
        }
        
        foreach (string path in searchDirectories) {
            if (Directory.Exists(path)) {
                existingDirectories.Add(path);
            }
        }

        return existingDirectories;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Find the maya application at the passed dir.
    /// Returns null if not found
    /// </summary>
    /// <returns>The maya application path.</returns>
    /// <param name="dir">Directory to be searched</param>
    private static string FindMayaAppPathInDirectory(string dir) {
        string path = dir;
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                path+= "/bin/maya.exe";
                break;
            }
            case RuntimePlatform.OSXEditor: {
                // MAYA_LOCATION on mac is set by Autodesk to be the
                // Contents directory. But let's make it easier on people
                // and allow just having it be the app bundle or a
                // directory that holds the app bundle.
                if (dir.EndsWith(".app/Contents")) {
                    path+= "/MacOS/Maya";
                } else if (dir.EndsWith(".app")) {
                    path+= "/Contents/MacOS/Maya";
                } else {
                    path+= "/Maya.app/Contents/MacOS/Maya";
                }

                break;
            }
            default:
                throw new NotImplementedException ();
        }

        return File.Exists(path) ? path : null;
    }

//----------------------------------------------------------------------------------------------------------------------

    private static string Find3DSMaxAppPathInDirectory(string dir) {
        //[TODO-sin: 2020-4-27] Implement this
        throw new System.NotImplementedException();
    }

    
//----------------------------------------------------------------------------------------------------------------------

    //May return null if no app is detected
    private static string FindDCCToolAppPathInDirectory(DCCToolInfo info, string dir) {
        switch (info.Type) {
            case DCCToolType.AUTODESK_MAYA: {
                return FindMayaAppPathInDirectory(dir);
            }
            case DCCToolType.AUTODESK_3DSMAX: {
                return Find3DSMaxAppPathInDirectory(dir);
            }
        }

        return null;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Find DCC Tools by searching default installation folders, and looking at default environment variables.
    /// </summary>
    internal static Dictionary<string, DCCToolInfo> FindInstalledDCCTools() {
        List<string> vendorDirs = GetDefaultVendorDirectories();
        Dictionary<string, DCCToolInfo> dccPaths = new Dictionary<string, DCCToolInfo>();
        
        //From default Folders
        foreach (string vendorDir in vendorDirs) {
            foreach (var dcc in DEFAULT_DCC_TOOLS_BY_FOLDER) {
                string dir = Path.Combine(vendorDir, dcc.Key);
                if (!Directory.Exists(dir))
                    continue;

                string appPath = FindDCCToolAppPathInDirectory(dcc.Value, dir);

                if (string.IsNullOrEmpty(appPath)) {
                    continue;
                }
                
                dccPaths.Add(appPath, new DCCToolInfo(dcc.Value));
            }
        }
        
        //From default environment vars:
        foreach (var dcc in DEFAULT_DCC_TOOLS_BY_ENV_VAR) {
            string dir = Environment.GetEnvironmentVariable(dcc.Key);
            if (string.IsNullOrEmpty(dir) || !Directory.Exists(dir))
                continue;
            
            string appPath = FindDCCToolAppPathInDirectory(dcc.Value, dir);
            if (string.IsNullOrEmpty(appPath)) {
                continue;
            }
                
            dccPaths.Add(appPath, new DCCToolInfo(dcc.Value));
        }
        
        return dccPaths;
    }
    

//----------------------------------------------------------------------------------------------------------------------    
    
    //key: default folder name
    static readonly Dictionary<string, DCCToolInfo> DEFAULT_DCC_TOOLS_BY_FOLDER = new Dictionary<string, DCCToolInfo>() {
        { "maya2017", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2017" ) },
        { "maya2018", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2018" ) },
        { "maya2019", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2019" ) },
        { "maya2020", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2020" ) },
        { "3ds Max 2017", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2017" ) },
        { "3ds Max 2018", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2018" ) },
        { "3ds Max 2019", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2019" ) },
        { "3ds Max 2020", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2020" ) },
    };
    
    //environment variables
    static readonly Dictionary<string, DCCToolInfo> DEFAULT_DCC_TOOLS_BY_ENV_VAR = new Dictionary<string, DCCToolInfo>() {
        { "MAYA_LOCATION_2017", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2017" ) },
        { "MAYA_LOCATION_2018", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2018" ) },
        { "MAYA_LOCATION_2019", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2019" ) },
        { "MAYA_LOCATION_2020", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2020" ) },
        { "ADSK_3DSMAX_SDK_2017", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2017" ) },
        { "ADSK_3DSMAX_SDK_2018", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2018" ) },
        { "ADSK_3DSMAX_SDK_2019", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2019" ) },
        { "ADSK_3DSMAX_SDK_2020", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2020" ) },
    };

    
}

}