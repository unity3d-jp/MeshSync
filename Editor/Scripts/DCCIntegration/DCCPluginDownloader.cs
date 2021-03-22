using System;
using System.Collections.Generic;
using System.ComponentModel;
using UnityEngine;
using System.Net;
using Unity.FilmInternalUtilities;
using System.IO;
using JetBrains.Annotations;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using PackageInfo = UnityEditor.PackageManager.PackageInfo;

namespace Unity.MeshSync.Editor {


internal class DCCPluginDownloader  {

    //dccPlatformNames example: Maya_Windows.zip
    internal DCCPluginDownloader(bool showProgressBar, string destFolder, string[] dccPlatformNames) {
        m_showProgressBar = showProgressBar;
        m_destFolder = destFolder;
        m_dccPlatformNames = new Queue<string>(dccPlatformNames);
        m_finishedDCCPluginLocalPaths = new List<string>();
    } 
    
//----------------------------------------------------------------------------------------------------------------------    
    
    internal void Execute(string requestedVersion, Action<string, List<string>> onSuccess, Action onFail) {
        
        m_finishedDCCPluginLocalPaths.Clear();
        
        //Try to get the files from the package. If failed, then handle it
        DisplayProgressBar("Copying MeshSync DCC Plugins","",0);
        CopyDCCPluginsFromPackage(requestedVersion, 
            
            /*onSuccess=*/ (string installedVersion) => {
                ClearProgressBar();
                onSuccess(installedVersion, m_finishedDCCPluginLocalPaths);
            
            }, 
            /*onFail=*/ (string version) => {
                ClearProgressBar();
                onFail();
            }
        );
     }
     
     
//----------------------------------------------------------------------------------------------------------------------        
    void CopyDCCPluginsFromPackage(string dccPluginVersion, Action<string> onSuccess, Action<string> onFail) 
    {
        PackageRequestJobManager.CreateListRequest(/*offlineMode=*/true, /*includeIndirectIndependencies=*/ true, 
            /*onSuccess=*/ (listReq) =>{
                PackageInfo packageInfo       = listReq.FindPackage(MESHSYNC_DCC_PLUGIN_PACKAGE);
                if (null != packageInfo && packageInfo.version==dccPluginVersion) {
                    //Package is already installed.
                    CopyDCCPluginsFromPackage(dccPluginVersion);
                    onSuccess(packageInfo.version);
                    return;
                }

                string packageNameAndVer = $"{MESHSYNC_DCC_PLUGIN_PACKAGE}@{dccPluginVersion}";
                PackageRequestJobManager.CreateAddRequest(packageNameAndVer, 
                    /*onSuccess=*/ (addReq) => {
                        //Package was successfully added
                        CopyDCCPluginsFromPackage(dccPluginVersion);                   
                        onSuccess(dccPluginVersion);
                    }, 
                    /*onFail=*/ (req) => {
                        PackageInfo meshSyncInfo = listReq.FindPackage(MeshSyncConstants.PACKAGE_NAME);
                        onFail?.Invoke(meshSyncInfo.version);
                    });
            }, 
            /*OnFail=*/ null
        );
        
    }
       
//----------------------------------------------------------------------------------------------------------------------        
    void CopyDCCPluginsFromPackage(string version) {

        foreach (string dccPlatformName in m_dccPlatformNames) {

            //Check several folders
            string[] folders = {
                $"Packages/{MESHSYNC_DCC_PLUGIN_PACKAGE}/Editor/Plugins~",
                $"Library/PackageCache/{MESHSYNC_DCC_PLUGIN_PACKAGE}@{version}/Editor/Plugins~",                
            };
            
            string[] zipFileNames = {
                "UnityMeshSync_" + version + "_" + dccPlatformName,
                "UnityMeshSync_" + dccPlatformName,
            };

            string srcZipFilePath = null;

            foreach (string folder in folders) {
                foreach (string zipFileName in zipFileNames) {
                    string path = Path.GetFullPath(Path.Combine(folder, zipFileName));
                    if (!File.Exists(path)) {
                        continue;
                    }

                    srcZipFilePath = path;
                    break;
                }

                if (!string.IsNullOrEmpty(srcZipFilePath))
                    break;
            }

            if (string.IsNullOrEmpty(srcZipFilePath)) {
                continue;
            }

            Directory.CreateDirectory(m_destFolder);
            string dest = Path.Combine(m_destFolder,"UnityMeshSync_" + version + "_" + dccPlatformName);
            File.Copy(srcZipFilePath, dest, /*overwrite=*/true);
            m_finishedDCCPluginLocalPaths.Add(dest);
        }
        
        
    }


    
//----------------------------------------------------------------------------------------------------------------------        
    #region ProgressBar
    void DisplayProgressBar(string title, string info, float progress) {
        if (!m_showProgressBar)
            return;
        
        EditorUtility.DisplayProgressBar(title, info, progress);
        
        
    }

    bool DisplayCancelableProgressBar(string title, string info,float progress) {
        if (!m_showProgressBar)
            return false;

        return EditorUtility.DisplayCancelableProgressBar(title, info, progress);

    }

    void ClearProgressBar() {
        if (!m_showProgressBar)
            return;
        EditorUtility.ClearProgressBar();
    }

    
    #endregion
    
    
//----------------------------------------------------------------------------------------------------------------------        

    private readonly bool m_showProgressBar;
    private readonly string m_destFolder;
    private readonly Queue<string> m_dccPlatformNames;
    private readonly List<string> m_finishedDCCPluginLocalPaths;
        
    private const string MESHSYNC_DCC_PLUGIN_PACKAGE = "com.unity.meshsync.dcc-plugins";
}

} //end namespace

