using System;
using System.Collections.Generic;
using System.ComponentModel;
using UnityEngine;
using System.Net;
using Unity.AnimeToolbox;
using System.IO;
using JetBrains.Annotations;
using Unity.AnimeToolbox.Editor;
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
        
        //Try to get the files from package first. If failed, then try downloading directly
        DisplayProgressBar("Copying MeshSync DCC Plugins","",0);
        CopyDCCPluginsFromPackage(requestedVersion, 
            
            /*onSuccess=*/ (string installedVersion) => {
                ClearProgressBar();
                onSuccess(installedVersion, m_finishedDCCPluginLocalPaths);
            
            }, 
            /*onFail=*/ (string version) => {

                //Getting files from the package has failed. Download directly
                DownloadDCCPlugins(version, (string downloadedVersion)=> {
                    ClearProgressBar();
                    onSuccess(downloadedVersion, m_finishedDCCPluginLocalPaths);
                }, ()=> {
                    ClearProgressBar();
                    onFail();
                });
            }
        );
     }
     
     
//----------------------------------------------------------------------------------------------------------------------        
    void CopyDCCPluginsFromPackage(string dccPluginVersion, Action<string> onSuccess, Action<string> onFail) 
    {
        RequestJobManager.CreateListRequest(/*offlineMode=*/true, /*includeIndirectIndependencies=*/ true, 
            /*onSuccess=*/ (listReq) =>{
                PackageInfo packageInfo       = listReq.FindPackage(MESHSYNC_DCC_PLUGIN_PACKAGE);
                if (null != packageInfo && packageInfo.version==dccPluginVersion) {
                    //Package is already installed.
                    CopyDCCPluginsFromPackage(dccPluginVersion);
                    onSuccess(packageInfo.version);
                    return;
                }

                string packageNameAndVer = $"{MESHSYNC_DCC_PLUGIN_PACKAGE}@{dccPluginVersion}";
                RequestJobManager.CreateAddRequest(packageNameAndVer, 
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
    
//-------------------------------------------------1---------------------------------------------------------------------

    private void DownloadDCCPlugins(string version, Action<string> onComplete, Action onFail) 
    {
        Directory.CreateDirectory(m_destFolder);
        WebClient client = new WebClient();
        int initialQueueCount = m_dccPlatformNames.Count;

        //meta can be null when we failed to download it
        DCCPluginMeta meta = GetOrDownloadDCCPluginMeta(version);

        //Prepare WebClient
        client.DownloadFileCompleted += (object sender, AsyncCompletedEventArgs e) => {
            DCCPluginDownloadInfo lastInfo = new DCCPluginDownloadInfo(version, m_dccPlatformNames.Peek(), m_destFolder);
            if (e.Error != null) {
                if (File.Exists(lastInfo.LocalFilePath)) {
                    File.Delete(lastInfo.LocalFilePath);
                }

                //Try downloading using the latest known version to work.
                if (version != LATEST_KNOWN_VERSION) {
                 
                    DownloadDCCPlugins(LATEST_KNOWN_VERSION, onComplete, onFail);
                } else {
                    onFail();
                }
                return;
            }

            //Remove the finished one from queue
            m_dccPlatformNames.Dequeue();
            m_finishedDCCPluginLocalPaths.Add(lastInfo.LocalFilePath);

            
            DCCPluginDownloadInfo nextInfo = FindNextPluginToDownload(meta, version);
            if (null == nextInfo) {
                onComplete(version);
            } else {
                
                //Download the next one  
                client.DownloadFileAsync(new Uri(nextInfo.URL), nextInfo.LocalFilePath);
            }

        };

        client.DownloadProgressChanged += (object sender, DownloadProgressChangedEventArgs e) =>
        {
            float inverseNumPlugins = 1.0f / initialQueueCount;
            int curIndex = initialQueueCount - m_dccPlatformNames.Count;
            float curFileProgress = (curIndex) * inverseNumPlugins;
            float nextFileProgress = (curIndex+1) * inverseNumPlugins;

            float progress = curFileProgress + (e.ProgressPercentage * 0.01f * (nextFileProgress - curFileProgress));
            if(DisplayCancelableProgressBar("Downloading MeshSync DCC Plugins", m_dccPlatformNames.Peek(), progress)) 
            {
                client.CancelAsync();
            }
        };

        
        DCCPluginDownloadInfo downloadInfo = FindNextPluginToDownload(meta, version);

        if (null == downloadInfo) {
            onComplete(version);
            return;
        }

        //Execute downloading
        DisplayProgressBar("Downloading MeshSync DCC Plugins",m_dccPlatformNames.Peek(),0);
        client.DownloadFileAsync(new Uri(downloadInfo.URL), downloadInfo.LocalFilePath);

    }

//----------------------------------------------------------------------------------------------------------------------    
    DCCPluginDownloadInfo FindNextPluginToDownload(DCCPluginMeta meta, string version) {
        
        DCCPluginDownloadInfo ret = null;

        while (m_dccPlatformNames.Count > 0 && null == ret) {
            DCCPluginDownloadInfo downloadInfo = new DCCPluginDownloadInfo(version, 
                m_dccPlatformNames.Peek(), m_destFolder);
            if (null!=meta && File.Exists(downloadInfo.LocalFilePath)) {
                
                //Check MD5
                string md5 = FileUtility.ComputeFileMD5(downloadInfo.LocalFilePath);
                DCCPluginSignature signature = meta.GetSignature(Path.GetFileName(downloadInfo.LocalFilePath));
                if (null == signature || signature.MD5 != md5) {
                    ret = downloadInfo;
                } else {
                    //The same file has been downloaded. Skip.
                    m_dccPlatformNames.Dequeue();
                    m_finishedDCCPluginLocalPaths.Add(downloadInfo.LocalFilePath);
                }

            } else {
                ret = downloadInfo;
            }
        }

        return ret;
    }


//----------------------------------------------------------------------------------------------------------------------        

    //Download meta file synchronously
    [CanBeNull]
    DCCPluginMeta GetOrDownloadDCCPluginMeta(string version) {
        
        DCCPluginMeta ret = null;
        string metaURL = MeshSyncEditorConstants.DCC_PLUGINS_GITHUB_RELEASE_URL + version + "/meta.txt";
        string localPath = Path.Combine(m_destFolder, "meta_" + version + ".txt");
        if (File.Exists(localPath)) {
            ret = FileUtility.DeserializeFromJson<DCCPluginMeta>(localPath);
        }

        if (null != ret) {
            return ret;
        }
        
        WebClient client = new WebClient();
        try {
            client.DownloadFile(new Uri(metaURL), localPath);
            ret = FileUtility.DeserializeFromJson<DCCPluginMeta>(localPath);
        }
        catch {
            Debug.LogWarning("[MeshSync] Meta info can't be downloaded from: " + metaURL);
            if (File.Exists(localPath)) {
                File.Delete(localPath);
            }
        }


        return ret;

    }
    
//----------------------------------------------------------------------------------------------------------------------        
    void CopyDCCPluginsFromPackage(string version) {

        foreach (string dccPlatformName in m_dccPlatformNames) {
            const string DCC_PLUGIN_SRC_FOLDER = "Packages/" + MESHSYNC_DCC_PLUGIN_PACKAGE + "/Editor/Plugins~";

            string[] zipFileNames = {
                "UnityMeshSync_" + version + "_" + dccPlatformName,
                "UnityMeshSync_" + dccPlatformName,
            };

            string srcZipFilePath = null;
            foreach (string zipFileName in zipFileNames) {
                string path = Path.GetFullPath(Path.Combine(DCC_PLUGIN_SRC_FOLDER, zipFileName));
                if (!File.Exists(path)) {
                    continue;
                }

                srcZipFilePath = path;
                break;
            }

            if (string.IsNullOrEmpty(srcZipFilePath)) {
                continue;
            }

            
            DCCPluginDownloadInfo downloadInfo = new DCCPluginDownloadInfo(version, dccPlatformName, m_destFolder);
            File.Copy(srcZipFilePath, downloadInfo.LocalFilePath, /*overwrite=*/true);
            m_finishedDCCPluginLocalPaths.Add(downloadInfo.LocalFilePath);
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
        
    const string LATEST_KNOWN_VERSION = "0.6.0-preview";
    private const string MESHSYNC_DCC_PLUGIN_PACKAGE = "com.unity.meshsync.dcc-plugins";
}

} //end namespace

