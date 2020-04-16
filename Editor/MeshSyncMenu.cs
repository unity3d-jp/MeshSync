using System;
using System.ComponentModel;
using UnityEngine;
using System.Net;
using Unity.AnimeToolbox;
using System.IO;


namespace UnityEditor.MeshSync {

public static class MeshSyncMenu  {

    
     [MenuItem("Assets/MeshSync/Download DCC Plugins", false, 100)]
     static void DownloadDCCPlugins() {
         
         string destFolder = EditorUtility.OpenFolderPanel("Select copy destination", "", "");
         if (string.IsNullOrEmpty(destFolder))
             return;

         EditorUtility.DisplayProgressBar("Copying MeshSync DCC Plugins","",0);
         TryCopyDCCPluginsFromPackage(destFolder, (version) => {
             DownloadDCCPlugins(version, destFolder);
         });
         EditorUtility.ClearProgressBar();
         
     }
     
//----------------------------------------------------------------------------------------------------------------------        
    static void TryCopyDCCPluginsFromPackage(string destFolder, Action<string> onFail) {
        RequestJobManager.CreateListRequest(true,true, (listReq) => {
            PackageManager.PackageInfo packageInfo = listReq.FindPackage(MESHSYNC_DCC_PLUGIN_PACKAGE);
            if (null != packageInfo) {
                //Package is already installed.
                CopyDCCPluginsFromPackage(destFolder);
                return;
            }

            RequestJobManager.CreateAddRequest(MESHSYNC_DCC_PLUGIN_PACKAGE, (addReq) => {
                //Package was successfully added
                CopyDCCPluginsFromPackage(destFolder);
            }, (req) => {
                PackageManager.PackageInfo meshSyncInfo = listReq.FindPackage(MESHSYNC_PACKAGE);
                onFail?.Invoke(meshSyncInfo.version);
            });
        }, null);
        
    }
    
//-------------------------------------------------1---------------------------------------------------------------------

     static void DownloadDCCPlugins(string version, string destFolder) {
         //Sample link:
         string[] plugins = {
             "3DSMAX_Windows.zip",
             "Blender_Linux.zip",
             "Blender_Mac.zip",
             "Blender_Windows.zip",
             "Maya_Linux.zip",
             "Maya_Mac.zip",
             "Maya_Windows.zip",
             "Metasequoia_Mac.zip",
             "Metasequoia_Windows.zip",
             "MotionBuilder_Linux.zip",
             "MotionBuilder_Windows.zip",
         };

         Directory.CreateDirectory(destFolder);
         int numPlugins = plugins.Length;
         int curPluginIndex = 0;
         WebClient client = new WebClient();


         //Prepare WebClient
         client.DownloadFileCompleted += (object sender, AsyncCompletedEventArgs e) => {
             if (e.Error != null) {
                 
                 DCCPluginDownloadInfo lastInfo = new DCCPluginDownloadInfo(version, plugins[curPluginIndex], destFolder);
                 if (File.Exists(lastInfo.FilePath)) {
                     File.Delete(lastInfo.FilePath);
                 }
                 
                 //Try downloading using the latest known version to work.
                 if (version != LATEST_KNOWN_VERSION) {
                     
                     DownloadDCCPlugins(LATEST_KNOWN_VERSION, destFolder);
                 } else {
                     Debug.LogError("Failed to download DCC plugins. URL: " 
                         + lastInfo.URL + ". Error: " + e.Error);
                     EditorUtility.ClearProgressBar();
                 }
                 return;
             }
             

             ++curPluginIndex;
             
             //if finished downloading all files
             if (curPluginIndex >= numPlugins) {
                 EditorUtility.RevealInFinder(destFolder);
                 EditorUtility.ClearProgressBar();
                 Debug.Log("Downloaded MeshSync DCC Plugins to " + destFolder);
             } else {
               //Download the next one  
               DCCPluginDownloadInfo curInfo = new DCCPluginDownloadInfo(version, plugins[curPluginIndex], destFolder);
               client.DownloadFileAsync(new Uri(curInfo.URL), curInfo.FilePath);
             }
         };
         
         client.DownloadProgressChanged += (object sender, DownloadProgressChangedEventArgs e) =>
         {
             float inverseNumPlugins = 1.0f / numPlugins;
             float curFileProgress = (curPluginIndex) * inverseNumPlugins;
             float nextFileProgress = (curPluginIndex+1) * inverseNumPlugins;

             float progress = curFileProgress + (e.ProgressPercentage * 0.01f * (nextFileProgress - curFileProgress));
             if(EditorUtility.DisplayCancelableProgressBar("Downloading MeshSync DCC Plugins", 
                 plugins[curPluginIndex], progress)) 
             {
                 client.CancelAsync();
             }
         };


         //Execute downloading recursively
         EditorUtility.DisplayProgressBar("Downloading MeshSync DCC Plugins",plugins[curPluginIndex],0);
         DCCPluginDownloadInfo downloadInfo = new DCCPluginDownloadInfo(version, plugins[curPluginIndex], destFolder);
         client.DownloadFileAsync(new Uri(downloadInfo.URL), downloadInfo.FilePath);

     }


    
//----------------------------------------------------------------------------------------------------------------------        
    static void CopyDCCPluginsFromPackage(string destFolder) {
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

    const string LATEST_KNOWN_VERSION = "0.0.3-preview";
    private const string MESHSYNC_PACKAGE = "com.unity.meshsync";
    private const string MESHSYNC_DCC_PLUGIN_PACKAGE = "com.unity.meshsync-dcc-plugins";
}

} //end namespace

