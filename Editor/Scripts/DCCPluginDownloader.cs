using System;
using System.Collections.Generic;
using System.ComponentModel;
using UnityEngine;
using System.Net;
using Unity.AnimeToolbox;
using System.IO;
using System.Security.Cryptography;

namespace UnityEditor.MeshSync {


internal class DCCPluginDownloader  {

    DCCPluginDownloader(bool showProgressBar) {
        m_showProgressBar = showProgressBar;
    } 
    
//----------------------------------------------------------------------------------------------------------------------    
    
    internal void DownloadDCCPlugins(string destFolder, Queue<string> dccPlatformNames, Action onSuccess, Action onFail) {

        Action onSuccessAndCleanUp = () => {
            ClearProgressBar();
            onSuccess();
        };
        
        Action onFailAndCleanUp = () => {
            ClearProgressBar();
            onFail();
        };
        
        //Try to get the files from package first. If failed, then try downloading directly
        DisplayProgressBar("Copying MeshSync DCC Plugins","",0);
        TryCopyDCCPluginsFromPackage(destFolder,dccPlatformNames, onSuccessAndCleanUp, (version) => {

            //Getting files from the package has failed. Download directly
            DownloadDCCPlugins(version, destFolder, dccPlatformNames, onSuccessAndCleanUp, onFailAndCleanUp);
        });
     }
     
     
//----------------------------------------------------------------------------------------------------------------------        
    void TryCopyDCCPluginsFromPackage(string destFolder, Queue<string> dccPlatformNames, 
        Action onSuccess, Action<string> onFail) 
    {
        RequestJobManager.CreateListRequest(true,true, (listReq) => {
            PackageManager.PackageInfo packageInfo = listReq.FindPackage(MESHSYNC_DCC_PLUGIN_PACKAGE);
            if (null != packageInfo) {
                //Package is already installed.
                CopyDCCPluginsFromPackage(destFolder, dccPlatformNames);
                onSuccess();
                return;
            }

            RequestJobManager.CreateAddRequest(MESHSYNC_DCC_PLUGIN_PACKAGE, (addReq) => {
                //Package was successfully added
                CopyDCCPluginsFromPackage(destFolder, dccPlatformNames);
                onSuccess();
            }, (req) => {
                PackageManager.PackageInfo meshSyncInfo = listReq.FindPackage(MESHSYNC_PACKAGE);
                onFail?.Invoke(meshSyncInfo.version);
            });
        }, null);
        
    }
    
//-------------------------------------------------1---------------------------------------------------------------------

    void DownloadDCCPlugins(string version, string destFolder, Queue<string> dccPlatformNames, 
        Action onComplete, Action onFail) 
    {
        Directory.CreateDirectory(destFolder);
        WebClient client = new WebClient();
        int initialQueueCount = dccPlatformNames.Count;

        //meta can be null when we failed to download it
        DCCPluginMeta meta = TryDownloadDCCPluginMeta(version);

        //Prepare WebClient
        client.DownloadFileCompleted += (object sender, AsyncCompletedEventArgs e) => {
            if (e.Error != null) {


                DCCPluginDirectDownloadInfo lastInfo = new DCCPluginDirectDownloadInfo(version, dccPlatformNames.Peek(), destFolder);
                if (File.Exists(lastInfo.LocalFilePath)) {
                    File.Delete(lastInfo.LocalFilePath);
                }

                //Try downloading using the latest known version to work.
                if (version != LATEST_KNOWN_VERSION) {
                 
                    DownloadDCCPlugins(LATEST_KNOWN_VERSION, destFolder, dccPlatformNames, onComplete, onFail);
                } else {
                    onFail();
                }
                return;
            }

            //Remove the finished one from queue
            dccPlatformNames.Dequeue();

            
            DCCPluginDirectDownloadInfo nextInfo = FindNextPluginToDownload(meta, version, destFolder, dccPlatformNames);
            if (null == nextInfo) {
                onComplete();
            } else {
                
                //Download the next one  
                client.DownloadFileAsync(new Uri(nextInfo.URL), nextInfo.LocalFilePath);
            }

        };

        client.DownloadProgressChanged += (object sender, DownloadProgressChangedEventArgs e) =>
        {
            float inverseNumPlugins = 1.0f / initialQueueCount;
            int curIndex = initialQueueCount - dccPlatformNames.Count;
            float curFileProgress = (curIndex) * inverseNumPlugins;
            float nextFileProgress = (curIndex+1) * inverseNumPlugins;

            float progress = curFileProgress + (e.ProgressPercentage * 0.01f * (nextFileProgress - curFileProgress));
            if(DisplayCancelableProgressBar("Downloading MeshSync DCC Plugins", dccPlatformNames.Peek(), progress)) 
            {
                client.CancelAsync();
            }
        };

        
        DCCPluginDirectDownloadInfo directDownloadInfo = FindNextPluginToDownload(meta, version, destFolder, dccPlatformNames);

        if (null == directDownloadInfo) {
            onComplete();
            return;
        }

        //Execute downloading
        DisplayProgressBar("Downloading MeshSync DCC Plugins",dccPlatformNames.Peek(),0);
        client.DownloadFileAsync(new Uri(directDownloadInfo.URL), directDownloadInfo.LocalFilePath);

    }

//----------------------------------------------------------------------------------------------------------------------    
    static DCCPluginDirectDownloadInfo FindNextPluginToDownload(DCCPluginMeta meta, string version, 
        string destFolder, 
        Queue<string> dccPlatformNames) {
        
        DCCPluginDirectDownloadInfo ret = null;

        while (dccPlatformNames.Count > 0 && null == ret) {
            DCCPluginDirectDownloadInfo directDownloadInfo = new DCCPluginDirectDownloadInfo(version, dccPlatformNames.Peek(), destFolder);
            if (null!=meta && File.Exists(directDownloadInfo.LocalFilePath)) {
                
                //Check MD5
                string md5 = ComputeFileMD5(directDownloadInfo.LocalFilePath);
                DCCPluginSignature signature = meta.GetSignature(Path.GetFileName(directDownloadInfo.LocalFilePath));
                if (signature.MD5 != md5) {
                    ret = directDownloadInfo;
                } else {
                    //The same file has been downloaded. Skip.
                    dccPlatformNames.Dequeue();
                }

            } else {
                ret = directDownloadInfo;
            }
        }

        return ret;
    }


//----------------------------------------------------------------------------------------------------------------------        

    //Download meta file synchronously
    static DCCPluginMeta TryDownloadDCCPluginMeta(string version) {
        string metaURL = MeshSyncEditorConstants.DCC_PLUGINS_GITHUB_RELEASE_URL + version + "/meta.txt";
        string tempPath = FileUtil.GetUniqueTempPathInProject();
        
        WebClient client = new WebClient();
        DCCPluginMeta ret = null;
        try {
            client.DownloadFile(new Uri(metaURL), tempPath);
            string json = File.ReadAllText(tempPath);
            ret = JsonUtility.FromJson<DCCPluginMeta>(json);
        }
        catch {
            Debug.LogWarning("[MeshSync] Meta info can't be downloaded from: " + metaURL);
        }

        if (File.Exists(tempPath)) {
            File.Delete(tempPath);
        }

        return ret;

    }
    
//----------------------------------------------------------------------------------------------------------------------        
    static void CopyDCCPluginsFromPackage(string destFolder, Queue<string> dccPlatformNames) {
        //[TODO-sin: 2020-4-8] Assume that package was successfully installed. Copy the plugins somewhere
        
    }

//----------------------------------------------------------------------------------------------------------------------        
    static string ComputeFileMD5(string path) {
        using (var md5 = MD5.Create()) {
            using (var stream = File.OpenRead(path)) {
                byte[] hash = md5.ComputeHash(stream);
                string str = BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                return str;
            }
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

    const string LATEST_KNOWN_VERSION = "0.1.0-preview";
    private const string MESHSYNC_PACKAGE = "com.unity.meshsync";
    private const string MESHSYNC_DCC_PLUGIN_PACKAGE = "com.unity.meshsync-dcc-plugins";
}

} //end namespace

