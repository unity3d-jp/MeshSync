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
        
        m_defaultPlayerConfigs = new MeshSyncPlayerConfig[(int) MeshSyncObjectType.NUM_TYPES]; 

        MeshSyncPlayerConfig config = new MeshSyncPlayerConfig();
        m_defaultPlayerConfigs[(int) MeshSyncObjectType.SERVER] = config;
        
        config = new MeshSyncPlayerConfig();
        m_defaultPlayerConfigs[(int) MeshSyncObjectType.CACHE_PLAYER] = config;
        
        //[TODO-sin: 2020-5-21] handle what happens if in loading we have less or more types ?
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


    [SerializeField] private MeshSyncPlayerConfig[] m_defaultPlayerConfigs;

}

} //end namespace