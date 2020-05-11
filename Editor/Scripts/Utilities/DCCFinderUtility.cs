using System;
using System.Collections.Generic;
using System.IO;
using JetBrains.Annotations;
using Unity.AnimeToolbox;
using UnityEngine;


namespace UnityEditor.MeshSync {
    
/// <summary>
/// A utility class for finding DCC Tools
/// </summary>
public static class DCCFinderUtility {
    
    //Find the location of supported DCC tools on Windows and Mac.
    //Currently only support Autodesk products
    private static List<string> GetDefaultVendorDirectories() {

        List<string> existingDirectories = new List<string>();
        List<string> searchDirectories = new List<string>();

        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                DriveInfo[] allDrives = DriveInfo.GetDrives();
                foreach (DriveInfo drive in allDrives) {
                    searchDirectories.Add(Path.Combine(drive.Name, @"Program Files\Autodesk") );
                }
                break;
            }
            case RuntimePlatform.OSXEditor: {
                searchDirectories.Add("/Applications/Autodesk");
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                searchDirectories.Add("~/");
                searchDirectories.Add("/usr/autodesk");
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
    /// <param name="dir">Directory to be searched</param>
    /// <param name="version">known Maya version</param>
    private static DCCToolInfo FindMayaInDirectory(string dir, string version) {
        string appPath = dir;
        string iconPath = dir;
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                appPath+= @"\bin\maya.exe";
                iconPath += @"\icons\mayaico.png";
                break;
            }
            case RuntimePlatform.OSXEditor: {
                // MAYA_LOCATION on mac is set by Autodesk to be the
                // Contents directory. But let's make it easier on people
                // and allow just having it be the app bundle or a
                // directory that holds the app bundle.
                if (dir.EndsWith(".app/Contents")) {
                    appPath+= "/MacOS/Maya";
                } else if (dir.EndsWith(".app")) {
                    appPath+= "/Contents/MacOS/Maya";
                } else {
                    appPath+= "/Maya.app/Contents/MacOS/Maya";
                }
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                appPath+= "/bin/maya";
                break;
            }
            default:
                throw new NotImplementedException ();
        }

        if (!File.Exists(appPath))
            return null;
        
        iconPath = (!File.Exists(iconPath)) ? null : iconPath;

        version = (string.IsNullOrEmpty(version)) ? FindMayaVersion(appPath) : version;

        return new DCCToolInfo(DCCToolType.AUTODESK_MAYA,version) {
            AppPath = appPath,
            IconPath = iconPath
        };
        
    }

//----------------------------------------------------------------------------------------------------------------------

    internal static string FindMayaVersion(string appPath) {
        
        string productDir = null;
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                
                //2 levels up: "/bin/maya.exe";
                productDir = PathUtility.TryGetDirectoryName(appPath, 2);
                break;
            }
            case RuntimePlatform.OSXEditor: {
                //4 levels up: "/Maya.app/Contents/MacOS/Maya";
                productDir = PathUtility.TryGetDirectoryName(appPath, 4);
        
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                //2 levels up:/usr/autodesk/maya2019/bin/maya
                productDir = PathUtility.TryGetDirectoryName(appPath, 2);
                break;
            }



            default:
                throw new NotImplementedException ();
        }

        if (string.IsNullOrEmpty(productDir)) {
            return "Unknown";
        }

        const string MAYA_STR = "maya";
        int index = productDir.IndexOf(MAYA_STR, StringComparison.OrdinalIgnoreCase);
        if (index == -1) {
            return "Unknown";
        }
        
        string version = productDir.Substring (index + MAYA_STR.Length);
        return version.Trim();
    }

//----------------------------------------------------------------------------------------------------------------------

    //3DS Max is only available for Windows
    private static DCCToolInfo Find3DSMaxInDirectory(string dir, string version) {
        if (RuntimePlatform.WindowsEditor != Application.platform) {
            return null;
        }
        
        string appPath = dir + @"\3dsmax.exe";
        string iconPath = dir + @"\Icons\icon_main.ico";
        
        if (!File.Exists(appPath))
            return null;
        
        iconPath = (!File.Exists(iconPath)) ? null : iconPath;       
        version = (string.IsNullOrEmpty(version)) ? Find3DSMaxVersion(appPath) : version;
        
        return new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX,version) {
            AppPath = appPath,
            IconPath = iconPath
        };
        
    }
//----------------------------------------------------------------------------------------------------------------------

    internal static string Find3DSMaxVersion(string appPath) {
        //4 levels up: "C:\Program Files\3dsMax 2019\3dsmax.exe";
        return PathUtility.TryGetDirectoryName(appPath, 1);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    internal static string FindMayaIcon(string appPath) {
        //4 levels up: "C:\Program Files\3dsMax 2019\3dsmax.exe";
        return PathUtility.TryGetDirectoryName(appPath, 1);
    }

//----------------------------------------------------------------------------------------------------------------------

    //Returns the DCCToolInfo of the DCC tool.
    //If version is null, then the correct version will be returned if found
    [CanBeNull]
    internal static DCCToolInfo FindDCCToolInDirectory(DCCToolType toolType, string version, string dir) {
        switch (toolType) {
            case DCCToolType.AUTODESK_MAYA: {
                return FindMayaInDirectory(dir, version);
            }
            case DCCToolType.AUTODESK_3DSMAX: {
                return Find3DSMaxInDirectory(dir, version);
            }
        }

        return null;
    }
    
//----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// Find DCC Tools by searching default installation folders, and looking at default environment variables.
    /// </summary>
    /// <returns>A dictionary containing the detected DCC tools, with their paths as keys</returns>
    public static Dictionary<string, DCCToolInfo> FindInstalledDCCTools() {
        List<string> vendorDirs = GetDefaultVendorDirectories();
        Dictionary<string, DCCToolInfo> dccPaths = new Dictionary<string, DCCToolInfo>();
        
        //From default Folders
        foreach (string vendorDir in vendorDirs) {
            foreach (var dcc in DEFAULT_DCC_TOOLS_BY_FOLDER) {
                string dir = Path.Combine(vendorDir, dcc.Key);
                if (!Directory.Exists(dir))
                    continue;
                DCCToolInfo dccToolInfo = dcc.Value;

                DCCToolInfo foundDCC = FindDCCToolInDirectory(dccToolInfo.Type, dccToolInfo.DCCToolVersion, dir);
                if (null == foundDCC)
                    continue;
                
                dccPaths.Add(foundDCC.AppPath, foundDCC);
            }
        }
        
        //From default environment vars:
        foreach (var dcc in DEFAULT_DCC_TOOLS_BY_ENV_VAR) {
            string dir = Environment.GetEnvironmentVariable(dcc.Key);
            if (string.IsNullOrEmpty(dir) || !Directory.Exists(dir))
                continue;
            DCCToolInfo dccToolInfo = dcc.Value;

            DCCToolInfo foundDCC = FindDCCToolInDirectory(dccToolInfo.Type, dccToolInfo.DCCToolVersion, dir);
            if (null == foundDCC)
                continue;
                
            dccPaths.Add(foundDCC.AppPath, foundDCC);
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