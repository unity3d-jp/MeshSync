using System;
using System.Collections.Generic;
using System.ComponentModel;
using UnityEngine;
using System.Net;
using Unity.AnimeToolbox;
using System.IO;


namespace UnityEditor.MeshSync {

internal static class MeshSyncMenu  {


    [MenuItem("Assets/MeshSync/Download DCC Plugins", false, 100)]
    static void DownloadDCCPlugins() {

        //Actual plugin name: UnityMeshSync_<version>_<postfix>
        Queue<string> pluginSuffixes = new Queue<string>( new[] {
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
        TryCopyDCCPluginsFromPackage(destFolder,pluginSuffixes, (version) => {

            //Getting files from the package has failed. Download directly
            DownloadDCCPlugins(version, destFolder, pluginSuffixes, true, () => {
                Debug.Log("Downloaded MeshSync DCC Plugins to " + destFolder);
                EditorUtility.RevealInFinder(destFolder);
            });

        });
     }
     
     
//----------------------------------------------------------------------------------------------------------------------        
    static void TryCopyDCCPluginsFromPackage(string destFolder, Queue<string> pluginSuffixes, Action<string> onFail) {
        EditorUtility.DisplayProgressBar("Copying MeshSync DCC Plugins","",0);
        RequestJobManager.CreateListRequest(true,true, (listReq) => {
            PackageManager.PackageInfo packageInfo = listReq.FindPackage(MESHSYNC_DCC_PLUGIN_PACKAGE);
            if (null != packageInfo) {
                //Package is already installed.
                CopyDCCPluginsFromPackage(destFolder, pluginSuffixes);
                return;
            }

            RequestJobManager.CreateAddRequest(MESHSYNC_DCC_PLUGIN_PACKAGE, (addReq) => {
                //Package was successfully added
                CopyDCCPluginsFromPackage(destFolder, pluginSuffixes);
            }, (req) => {
                PackageManager.PackageInfo meshSyncInfo = listReq.FindPackage(MESHSYNC_PACKAGE);
                onFail?.Invoke(meshSyncInfo.version);
            });
        }, null);
        
    }
    
//-------------------------------------------------1---------------------------------------------------------------------

    static void DownloadDCCPlugins(string version, string destFolder, Queue<string> pluginSuffixes, 
        bool skipExistingPlugins, Action onComplete) 
    {
        Directory.CreateDirectory(destFolder);
        WebClient client = new WebClient();
        int initialQueueCount = pluginSuffixes.Count;


        //Prepare WebClient
        client.DownloadFileCompleted += (object sender, AsyncCompletedEventArgs e) => {
            if (e.Error != null) {


                DCCPluginDownloadInfo lastInfo = new DCCPluginDownloadInfo(version, pluginSuffixes.Peek(), destFolder);
                if (File.Exists(lastInfo.FilePath)) {
                    File.Delete(lastInfo.FilePath);
                }

                //Try downloading using the latest known version to work.
                if (version != LATEST_KNOWN_VERSION) {
                 
                    DownloadDCCPlugins(LATEST_KNOWN_VERSION, destFolder, pluginSuffixes, skipExistingPlugins, onComplete);
                } else {
                    Debug.LogError("Failed to download DCC plugins. URL: " 
                     + lastInfo.URL + ". Error: " + e.Error);
                    EditorUtility.ClearProgressBar();
                }
                return;
            }

            //Remove the finished one from queue
            pluginSuffixes.Dequeue();

            
            DCCPluginDownloadInfo nextInfo = FindNextPluginToDownload(version, destFolder, pluginSuffixes, skipExistingPlugins);
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
            int curIndex = initialQueueCount - pluginSuffixes.Count;
            float curFileProgress = (curIndex) * inverseNumPlugins;
            float nextFileProgress = (curIndex+1) * inverseNumPlugins;

            float progress = curFileProgress + (e.ProgressPercentage * 0.01f * (nextFileProgress - curFileProgress));
            if(EditorUtility.DisplayCancelableProgressBar("Downloading MeshSync DCC Plugins", 
                pluginSuffixes.Peek(), progress)) 
            {
                client.CancelAsync();
            }
        };

        
        DCCPluginDownloadInfo downloadInfo = FindNextPluginToDownload(version, destFolder, pluginSuffixes, skipExistingPlugins);
        if (null == downloadInfo) {
            EditorUtility.ClearProgressBar();
            onComplete();
            return;
        }

        //Execute downloading
        EditorUtility.DisplayProgressBar("Downloading MeshSync DCC Plugins",pluginSuffixes.Peek(),0);
        client.DownloadFileAsync(new Uri(downloadInfo.URL), downloadInfo.FilePath);

    }

//----------------------------------------------------------------------------------------------------------------------    
    static DCCPluginDownloadInfo FindNextPluginToDownload(string version, string destFolder, 
        Queue<string> pluginSuffixes, bool skipExistingPlugins) {

        DCCPluginDownloadInfo ret = null;

        while (pluginSuffixes.Count > 0 && null == ret) {
            DCCPluginDownloadInfo downloadInfo = new DCCPluginDownloadInfo(version, pluginSuffixes.Peek(), destFolder);
            //[TODO-sin: 2020-5-1] Check MD5
            if (skipExistingPlugins && File.Exists(downloadInfo.FilePath)) {
                pluginSuffixes.Dequeue();
            } else {
                ret = downloadInfo;
            }
        }

        return ret;
    }

    
//----------------------------------------------------------------------------------------------------------------------        
    static void CopyDCCPluginsFromPackage(string destFolder, Queue<string> pluginSuffixes) {
        //[TODO-sin: 2020-4-8] Assume that package was successfully installed. Copy the plugins somewhere
        
    }


//----------------------------------------------------------------------------------------------------------------------        
    class DCCPluginDownloadInfo {
        public readonly string URL;
        public readonly string FilePath;

        //Sample link;
        //https://github.com/Unity-Technologies/MeshSyncDCCPlugin/releases/download/0.0.3-preview/UnityMeshSync_0.0.3-preview_3DSMAX_Windows.zip
        internal DCCPluginDownloadInfo(string version, string pluginName, string destFolder) {
            URL = "https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases/download/" 
                   + version + "/UnityMeshSync_" + version + "_" + pluginName;
            FilePath =  Path.Combine(destFolder,Path.GetFileName(URL));
            
        }

    }
    
//----------------------------------------------------------------------------------------------------------------------        

    const string LATEST_KNOWN_VERSION = "0.1.0-preview";
    private const string MESHSYNC_PACKAGE = "com.unity.meshsync";
    private const string MESHSYNC_DCC_PLUGIN_PACKAGE = "com.unity.meshsync-dcc-plugins";
}

} //end namespace

