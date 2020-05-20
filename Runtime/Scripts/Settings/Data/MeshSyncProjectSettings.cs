using System;
using System.IO;
using UnityEngine;


namespace Unity.MeshSync {

[Serializable]
internal class MeshSyncProjectSettings {

    internal static MeshSyncProjectSettings GetOrCreateSettings() {
        MeshSyncProjectSettings settings = LoadProjectSettings();
        if (null != settings) {
            return settings;
        }

        settings = new MeshSyncProjectSettings();
        settings.SaveProjectSettings();
        return settings;            
    }
//----------------------------------------------------------------------------------------------------------------------

    //Constructor
    MeshSyncProjectSettings() {
    }

    
//----------------------------------------------------------------------------------------------------------------------

    #region File Load/Save for Serialization/deserialization
    static MeshSyncProjectSettings LoadProjectSettings() {
        string path = MESHSYNC_PROJECT_SETTINGS_PATH;
        if (!File.Exists(path)) {
            return null;
        }
        
        string json = File.ReadAllText(path);
        MeshSyncProjectSettings settings = JsonUtility.FromJson<MeshSyncProjectSettings>(json);
        
        return settings;
    }
    
    void SaveProjectSettings() {
        Directory.CreateDirectory(Path.GetDirectoryName(MESHSYNC_PROJECT_SETTINGS_PATH));
        string json = JsonUtility.ToJson(this);
        File.WriteAllText(MESHSYNC_PROJECT_SETTINGS_PATH, json);
        
    }
    #endregion
    

//----------------------------------------------------------------------------------------------------------------------

    const string MESHSYNC_PROJECT_SETTINGS_PATH = "ProjectSettings/MeshSyncSettings.asset";

    


}

} //end namespace