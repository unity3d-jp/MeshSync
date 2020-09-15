using System.IO;

namespace Unity.MeshSync.Editor {


internal class DCCPluginDownloadInfo {
    public readonly string URL;
    public readonly string LocalFilePath;

    //Sample link;
    //https://github.com/Unity-Technologies/MeshSyncDCCPlugin/releases/download/0.0.3-preview/UnityMeshSync_0.0.3-preview_3DSMAX_Windows.zip
    internal DCCPluginDownloadInfo(string version, string pluginName, string destFolder) {
        URL = MeshSyncEditorConstants.DCC_PLUGINS_GITHUB_RELEASE_URL + version 
            + "/UnityMeshSync_" + version + "_" + pluginName;
        LocalFilePath =  Path.Combine(destFolder,Path.GetFileName(URL));
    }

}



} //end namespace

