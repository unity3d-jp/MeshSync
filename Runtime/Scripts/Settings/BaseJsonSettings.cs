using System;
using System.IO;
using Unity.AnimeToolbox;


namespace Unity.MeshSync {

[Serializable]
internal abstract class BaseJsonSettings  {
    
//----------------------------------------------------------------------------------------------------------------------
    
    internal bool SaveSettings() {
        string path = GetSettingsPath();
        string dir = Path.GetDirectoryName(path);
        if (!string.IsNullOrEmpty(dir)) {
            Directory.CreateDirectory(dir);

        }

        object objectLock = GetLock();

        lock (objectLock) {
            FileUtility.SerializeToJson(this, path);
        }

        return true;
    }   
    

//----------------------------------------------------------------------------------------------------------------------
    protected abstract object GetLock();
    internal abstract string GetSettingsPath();

//----------------------------------------------------------------------------------------------------------------------

}


} //end namespace