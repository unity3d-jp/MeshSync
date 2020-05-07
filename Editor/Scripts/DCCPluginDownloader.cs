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

    //dccPlatformNames example: Maya_Windows.zip
    internal DCCPluginDownloader(bool showProgressBar, string destFolder, string[] dccPlatformNames) {
        m_showProgressBar = showProgressBar;
        m_destFolder = destFolder;
        m_dccPlatformNames = new Queue<string>(dccPlatformNames);
    } 
    
//----------------------------------------------------------------------------------------------------------------------    
    
    internal void Execute(Action onSuccess, Action onFail) {

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
        TryCopyDCCPluginsFromPackage(onSuccessAndCleanUp, (version) => {

            //Getting files from the package has failed. Download directly
            DownloadDCCPlugins(version, onSuccessAndCleanUp, onFailAndCleanUp);
        });
     }
     
     
//----------------------------------------------------------------------------------------------------------------------        
    void TryCopyDCCPluginsFromPackage(Action onSuccess, Action<string> onFail) 
    {
        RequestJobManager.CreateListRequest(true,true, (listReq) => {
            PackageManager.PackageInfo packageInfo = listReq.FindPackage(MESHSYNC_DCC_PLUGIN_PACKAGE);
            if (null != packageInfo) {
                //Package is already installed.
                CopyDCCPluginsFromPackage();
                onSuccess();
                return;
            }

            RequestJobManager.CreateAddRequest(MESHSYNC_DCC_PLUGIN_PACKAGE, (addReq) => {
                //Package was successfully added
                CopyDCCPluginsFromPackage();
                onSuccess();
            }, (req) => {
                PackageManager.PackageInfo meshSyncInfo = listReq.FindPackage(MESHSYNC_PACKAGE);
                onFail?.Invoke(meshSyncInfo.version);
            });
        }, null);
        
    }
    
//-------------------------------------------------1---------------------------------------------------------------------

    void DownloadDCCPlugins(string version, Action onComplete, Action onFail) 
    {
        Directory.CreateDirectory(m_destFolder);
        WebClient client = new WebClient();
        int initialQueueCount = m_dccPlatformNames.Count;

        //meta can be null when we failed to download it
        DCCPluginMeta meta = GetOrDownloadDCCPluginMeta(version);

        //Prepare WebClient
        client.DownloadFileCompleted += (object sender, AsyncCompletedEventArgs e) => {
            if (e.Error != null) {


                DCCPluginDirectDownloadInfo lastInfo = new DCCPluginDirectDownloadInfo(version, m_dccPlatformNames.Peek(), m_destFolder);
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

            
            DCCPluginDirectDownloadInfo nextInfo = FindNextPluginToDownload(meta, version, m_destFolder, m_dccPlatformNames);
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
            int curIndex = initialQueueCount - m_dccPlatformNames.Count;
            float curFileProgress = (curIndex) * inverseNumPlugins;
            float nextFileProgress = (curIndex+1) * inverseNumPlugins;

            float progress = curFileProgress + (e.ProgressPercentage * 0.01f * (nextFileProgress - curFileProgress));
            if(DisplayCancelableProgressBar("Downloading MeshSync DCC Plugins", m_dccPlatformNames.Peek(), progress)) 
            {
                client.CancelAsync();
            }
        };

        
        DCCPluginDirectDownloadInfo directDownloadInfo = FindNextPluginToDownload(meta, version, m_destFolder, m_dccPlatformNames);

        if (null == directDownloadInfo) {
            onComplete();
            return;
        }

        //Execute downloading
        DisplayProgressBar("Downloading MeshSync DCC Plugins",m_dccPlatformNames.Peek(),0);
        client.DownloadFileAsync(new Uri(directDownloadInfo.URL), directDownloadInfo.LocalFilePath);

    }

//----------------------------------------------------------------------------------------------------------------------    
    static DCCPluginDirectDownloadInfo FindNextPluginToDownload(DCCPluginMeta meta, string version, 
        string destFolder, Queue<string> dccPlatformNames) 
    {
        
        DCCPluginDirectDownloadInfo ret = null;

        while (dccPlatformNames.Count > 0 && null == ret) {
            DCCPluginDirectDownloadInfo directDownloadInfo = new DCCPluginDirectDownloadInfo(version, dccPlatformNames.Peek(), destFolder);
            if (null!=meta && File.Exists(directDownloadInfo.LocalFilePath)) {
                
                //Check MD5
                string md5 = FileUtility.ComputeMD5(directDownloadInfo.LocalFilePath);
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
    static void CopyDCCPluginsFromPackage() {
        //[TODO-sin: 2020-4-8] Assume that package was successfully installed. Copy the plugins to m_destFolder
        
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
    
    const string LATEST_KNOWN_VERSION = "0.1.0-preview";
    private const string MESHSYNC_PACKAGE = "com.unity.meshsync";
    private const string MESHSYNC_DCC_PLUGIN_PACKAGE = "com.unity.meshsync-dcc-plugins";
}

} //end namespace

