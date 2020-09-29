using System;
using System.Collections.Generic;
using System.IO;
using JetBrains.Annotations;
using Unity.AnimeToolbox;
using UnityEngine;


namespace Unity.MeshSync.Editor {
    
/// <summary>
/// A utility class for finding DCC Tools
/// </summary>
public static class DCCFinderUtility {
    
    //Find the location of supported DCC tools on Windows and Mac.
    private static List<string> GetDefaultVendorDirectories() {

        List<string> existingDirectories = new List<string>();
        List<string> searchDirectories = new List<string>();

        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                DriveInfo[] allDrives = DriveInfo.GetDrives();
                foreach (DriveInfo drive in allDrives) {
                    searchDirectories.Add(Path.Combine(drive.Name, @"Program Files\Autodesk") );
                    searchDirectories.Add(Path.Combine(drive.Name, @"Program Files\Blender Foundation") );
                }
                break;
            }
            case RuntimePlatform.OSXEditor: {
                searchDirectories.Add("/Applications/Autodesk");
                searchDirectories.Add("/Applications");
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                
                searchDirectories.Add(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile));
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
                    appPath  += "/MacOS/Maya";
                    iconPath += "/icons/mayaico.png";
                } else if (dir.EndsWith(".app")) {
                    appPath  += "/Contents/MacOS/Maya";
                    iconPath += "/Contents/icons/mayaico.png";
                } else {
                    appPath  += "/Maya.app/Contents/MacOS/Maya";
                    iconPath += "/Maya.app/Contents/icons/mayaico.png";
                }
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                appPath  += "/bin/maya";
                iconPath += "/icons/mayaico.png";
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
                productDir = PathUtility.GetDirectoryName(appPath, 2);
                break;
            }
            case RuntimePlatform.OSXEditor: {
                //4 levels up: "/Maya.app/Contents/MacOS/Maya";
                productDir = PathUtility.GetDirectoryName(appPath, 4);
        
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                //2 levels up:/usr/autodesk/maya2019/bin/maya
                productDir = PathUtility.GetDirectoryName(appPath, 2);
                break;
            }

            default:
                throw new NotImplementedException ();
        }

        if (string.IsNullOrEmpty(productDir)) {
            return UNKNOWN_VERSION;
        }

        const string MAYA_STR = "maya";
        int index = productDir.IndexOf(MAYA_STR, StringComparison.OrdinalIgnoreCase);
        if (index == -1) {
            return UNKNOWN_VERSION;
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
        string folderName = Path.GetFileName(Path.GetDirectoryName(appPath));
        int len = folderName.Length;
        if (len > 4) {
            return folderName.Substring(len - 4, 4);
        }
        return folderName;
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    #region Blender
    private static DCCToolInfo FindBlenderInDirectory(string dir, string version) {
        string appPath = dir;
        string iconPath = dir;
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                appPath+= @"\blender.exe";
                iconPath = null;
                break;
            }
            case RuntimePlatform.OSXEditor: {
                const string CONTENTS_APP_PATH = "/MacOS/Blender";
                const string CONTENTS_ICON_PATH = "/Resources/blender icon.icns";

                if (dir.EndsWith(".app/Contents")) {
                    appPath  += CONTENTS_APP_PATH;
                    iconPath += CONTENTS_ICON_PATH;
                } else if (dir.EndsWith(".app")) {
                    appPath  += $"/Contents{CONTENTS_APP_PATH}";
                    iconPath += $"/Contents{CONTENTS_ICON_PATH}";
                } else {
                    appPath  += $"/Blender.app/Contents{CONTENTS_APP_PATH}";
                    iconPath += $"/Blender.app/Contents{CONTENTS_ICON_PATH}";
                }
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                appPath  += @"/blender";
                iconPath += @"/blender.svg";
                break;
            }
            default:
                throw new NotImplementedException ();
        }

        if (!File.Exists(appPath))
            return null;
        
        iconPath = (!File.Exists(iconPath)) ? null : iconPath;

        version = (string.IsNullOrEmpty(version)) ? FindBlenderVersion(appPath) : version;

        return new DCCToolInfo(DCCToolType.BLENDER,version) {
            AppPath = appPath,
            IconPath = iconPath
        };
        
    }

    private static string FindBlenderVersion(string appPath) {

        //Blender has a directory in one of its subfolders with the version as its name
        string versionParentDir = null;
        
        switch (Application.platform) {
            case RuntimePlatform.WindowsEditor: {
                //- C:\Program Files\Blender Foundation\Blender\2.80
                //- C:\Program Files\Blender Foundation\Blender 2.82\2.82
                versionParentDir = Path.GetDirectoryName(appPath);
                break;
            }
            case RuntimePlatform.OSXEditor: {
                //2 levels up: "/Blender.app/Contents/MacOS/Blender";
                string resourcesDir = PathUtility.GetDirectoryName(appPath, 2);
                versionParentDir = Path.Combine(resourcesDir, "Resources");
                break;
            }
            case RuntimePlatform.LinuxEditor: {
                //Example: /home/Unity/blender-2.82a-linux64/2.82
                versionParentDir = Path.GetDirectoryName(appPath);
                break;

            }

            default:
                throw new NotImplementedException ();
        }

        
        if (string.IsNullOrEmpty(versionParentDir)) {
            return UNKNOWN_VERSION;
        }
                
        foreach (string versionDir in Directory.EnumerateDirectories(versionParentDir)) {
            string dirName = Path.GetFileName(versionDir);
            if (Single.TryParse(dirName, out float number))
                return dirName;
        }
        
        return UNKNOWN_VERSION;
    }
    
    #endregion
    
//----------------------------------------------------------------------------------------------------------------------

    //Returns the DCCToolInfo of the DCC tool.
    //If version is null, then the correct version will be returned if found
    [CanBeNull]
    internal static DCCToolInfo FindDCCToolInDirectory(DCCToolType toolType, string version, string dir) {

#if UNITY_EDITOR_WIN
        dir = dir.Replace("/","\\");
#endif             
        
        switch (toolType) {
            case DCCToolType.AUTODESK_MAYA: {
                return FindMayaInDirectory(dir, version);
            }
            case DCCToolType.AUTODESK_3DSMAX: {
                return Find3DSMaxInDirectory(dir, version);
            }
            case DCCToolType.BLENDER: {
                return FindBlenderInDirectory(dir, version);
            }
            default:
                throw new NotImplementedException();
        }
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

#if   UNITY_EDITOR_WIN        
        { "Blender",      new DCCToolInfo(DCCToolType.BLENDER, null ) },
        { "Blender 2.81", new DCCToolInfo(DCCToolType.BLENDER, "2.81" ) },
        { "Blender 2.82", new DCCToolInfo(DCCToolType.BLENDER, "2.82" ) },
        { "Blender 2.83", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },
        { "Blender 2.90", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
#elif UNITY_EDITOR_OSX        
        { "Blender.app", new DCCToolInfo(DCCToolType.BLENDER, null ) },
        { "Blender/Blender.app", new DCCToolInfo(DCCToolType.BLENDER, null ) },
#elif UNITY_EDITOR_LINUX
        { "blender-2.79b-linux-glibc219-x86_64", new DCCToolInfo(DCCToolType.BLENDER, "2.79" ) },
        { "blender-2.80rc3-linux-glibc217-x86_64", new DCCToolInfo(DCCToolType.BLENDER, "2.80" ) },
        { "blender-2.81a-linux-glibc217-x86_64", new DCCToolInfo(DCCToolType.BLENDER, "2.81" ) },
        { "blender-2.82a-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.82" ) },
        { "blender-2.83.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.1-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.2-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.3-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.4-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.5-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.6-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },
        { "blender-2.90.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
        { "blender-2.90.1-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
           
#endif        
        
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

    private const string UNKNOWN_VERSION = "Unknown";

}

}