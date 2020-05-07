﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using UnityEngine;
using System.Net;
using Unity.AnimeToolbox;
using System.IO;
using System.Security.Cryptography;

namespace UnityEditor.MeshSync {


internal static class MeshSyncMenu  {

    
    [MenuItem("Assets/MeshSync/Download DCC Plugins", false, 100)]
    static void DownloadDCCPlugins() {

        //Actual plugin name: UnityMeshSync_<version>_<postfix>
        Queue<string> dccPlatformNames = new Queue<string>( new[] {
            "3DSMAX_Windows.zip",
            "Blender_Linux.zip",
            "Blender_Mac.zip",
            "Blender_Windows.zip",
            "Maya_Linux.zip",
            "Maya_Mac.zip",
            "Maya_Windows.zip",
            "Metasequoia_Windows.zip",
            "MotionBuilder_Linux.zip",
            "MotionBuilder_Windows.zip", 
        });

        string destFolder = EditorUtility.OpenFolderPanel("Select copy destination", "", "");
        if (string.IsNullOrEmpty(destFolder))
            return;

        //Try to get the files from package first. If failed, then try downloading directly
        TryCopyDCCPluginsFromPackage(destFolder,dccPlatformNames, (version) => {

            //Getting files from the package has failed. Download directly
            DownloadDCCPlugins(version, destFolder, dccPlatformNames, true, () => {
                Debug.Log("Downloaded MeshSync DCC Plugins to " + destFolder);
                EditorUtility.RevealInFinder(destFolder);
            });

        });
     }
     
     
//----------------------------------------------------------------------------------------------------------------------        
    static void TryCopyDCCPluginsFromPackage(string destFolder, Queue<string> dccPlatformNames, Action<string> onFail) {
        EditorUtility.DisplayProgressBar("Copying MeshSync DCC Plugins","",0);
        RequestJobManager.CreateListRequest(true,true, (listReq) => {
            PackageManager.PackageInfo packageInfo = listReq.FindPackage(MESHSYNC_DCC_PLUGIN_PACKAGE);
            if (null != packageInfo) {
                //Package is already installed.
                CopyDCCPluginsFromPackage(destFolder, dccPlatformNames);
                return;
            }

            RequestJobManager.CreateAddRequest(MESHSYNC_DCC_PLUGIN_PACKAGE, (addReq) => {
                //Package was successfully added
                CopyDCCPluginsFromPackage(destFolder, dccPlatformNames);
            }, (req) => {
                PackageManager.PackageInfo meshSyncInfo = listReq.FindPackage(MESHSYNC_PACKAGE);
                onFail?.Invoke(meshSyncInfo.version);
            });
        }, null);
        
    }
    
//-------------------------------------------------1---------------------------------------------------------------------

    static void DownloadDCCPlugins(string version, string destFolder, Queue<string> dccPlatformNames, 
        bool skipExistingPlugins, Action onComplete) 
    {
        Directory.CreateDirectory(destFolder);
        WebClient client = new WebClient();
        int initialQueueCount = dccPlatformNames.Count;

        //meta can be null when we failed to download it
        DCCPluginMeta meta = TryDownloadDCCPluginMeta(version);

        //Prepare WebClient
        client.DownloadFileCompleted += (object sender, AsyncCompletedEventArgs e) => {
            if (e.Error != null) {


                DCCPluginDownloadInfo lastInfo = new DCCPluginDownloadInfo(version, dccPlatformNames.Peek(), destFolder);
                if (File.Exists(lastInfo.FilePath)) {
                    File.Delete(lastInfo.FilePath);
                }

                //Try downloading using the latest known version to work.
                if (version != LATEST_KNOWN_VERSION) {
                 
                    DownloadDCCPlugins(LATEST_KNOWN_VERSION, destFolder, dccPlatformNames, skipExistingPlugins, onComplete);
                } else {
                    Debug.LogError("Failed to download DCC plugins. URL: " 
                     + lastInfo.URL + ". Error: " + e.Error);
                    EditorUtility.ClearProgressBar();
                }
                return;
            }

            //Remove the finished one from queue
            dccPlatformNames.Dequeue();

            
            DCCPluginDownloadInfo nextInfo = FindNextPluginToDownload(meta, version, destFolder, dccPlatformNames);
            if (null == nextInfo) {
                EditorUtility.ClearProgressBar();
                onComplete();
            } else {
                
                //Download the next one  
                client.DownloadFileAsync(new Uri(nextInfo.URL), nextInfo.FilePath);
            }

        };

        client.DownloadProgressChanged += (object sender, DownloadProgressChangedEventArgs e) =>
        {
            float inverseNumPlugins = 1.0f / initialQueueCount;
            int curIndex = initialQueueCount - dccPlatformNames.Count;
            float curFileProgress = (curIndex) * inverseNumPlugins;
            float nextFileProgress = (curIndex+1) * inverseNumPlugins;

            float progress = curFileProgress + (e.ProgressPercentage * 0.01f * (nextFileProgress - curFileProgress));
            if(EditorUtility.DisplayCancelableProgressBar("Downloading MeshSync DCC Plugins", 
                dccPlatformNames.Peek(), progress)) 
            {
                client.CancelAsync();
            }
        };

        
        DCCPluginDownloadInfo downloadInfo = FindNextPluginToDownload(meta, version, destFolder, dccPlatformNames);

        if (null == downloadInfo) {
            EditorUtility.ClearProgressBar();
            onComplete();
            return;
        }

        //Execute downloading
        EditorUtility.DisplayProgressBar("Downloading MeshSync DCC Plugins",dccPlatformNames.Peek(),0);
        client.DownloadFileAsync(new Uri(downloadInfo.URL), downloadInfo.FilePath);

    }

//----------------------------------------------------------------------------------------------------------------------    
    static DCCPluginDownloadInfo FindNextPluginToDownload(DCCPluginMeta meta, string version, 
        string destFolder, 
        Queue<string> dccPlatformNames) {
        
        DCCPluginDownloadInfo ret = null;

        while (dccPlatformNames.Count > 0 && null == ret) {
            DCCPluginDownloadInfo downloadInfo = new DCCPluginDownloadInfo(version, dccPlatformNames.Peek(), destFolder);
            if (null!=meta && File.Exists(downloadInfo.FilePath)) {
                
                //Check MD5
                string md5 = ComputeFileMD5(downloadInfo.FilePath);
                DCCPluginSignature signature = meta.GetSignature(Path.GetFileName(downloadInfo.FilePath));
                if (signature.MD5 != md5) {
                    ret = downloadInfo;
                } else {
                    //The same file has been downloaded. Skip.
                    dccPlatformNames.Dequeue();
                }

            } else {
                ret = downloadInfo;
            }
        }

        return ret;
    }


//----------------------------------------------------------------------------------------------------------------------        

    //Download meta file synchronously
    static DCCPluginMeta TryDownloadDCCPluginMeta(string version) {
        string metaURL = GITHUB_RELEASE_URL + version + "/meta.txt";
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
    class DCCPluginDownloadInfo {
        public readonly string URL;
        public readonly string FilePath;

        //Sample link;
        //https://github.com/Unity-Technologies/MeshSyncDCCPlugin/releases/download/0.0.3-preview/UnityMeshSync_0.0.3-preview_3DSMAX_Windows.zip
        internal DCCPluginDownloadInfo(string version, string pluginName, string destFolder) {
            URL = GITHUB_RELEASE_URL + version + "/UnityMeshSync_" + version + "_" + pluginName;
            FilePath =  Path.Combine(destFolder,Path.GetFileName(URL));
        }

    }

    
//----------------------------------------------------------------------------------------------------------------------        

    const string GITHUB_RELEASE_URL = "https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases/download/";
    const string LATEST_KNOWN_VERSION = "0.1.0-preview";
    private const string MESHSYNC_PACKAGE = "com.unity.meshsync";
    private const string MESHSYNC_DCC_PLUGIN_PACKAGE = "com.unity.meshsync-dcc-plugins";
}

} //end namespace

