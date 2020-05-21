using System;
using System.IO;
using UnityEngine;


namespace Unity.MeshSync {

[Serializable]
[Path(MESHSYNC_PROJECT_SETTINGS_PATH)]
internal class MeshSyncProjectSettings : SingletonJsonSettings<MeshSyncProjectSettings> {

//----------------------------------------------------------------------------------------------------------------------

    //Constructor
    private MeshSyncProjectSettings() {
        
        m_defaultPlayerConfigs = new MeshSyncPlayerConfig[(int) MeshSyncObjectType.NUM_TYPES]; 

        MeshSyncPlayerConfig config = new MeshSyncPlayerConfig();
        m_defaultPlayerConfigs[(int) MeshSyncObjectType.SERVER] = config;
        
        config = new MeshSyncPlayerConfig();
        m_defaultPlayerConfigs[(int) MeshSyncObjectType.CACHE_PLAYER] = config;
    }
   

//----------------------------------------------------------------------------------------------------------------------

    internal MeshSyncPlayerConfig GetDefaultPlayerConfig(MeshSyncObjectType objectType) {
        return m_defaultPlayerConfigs[(int) objectType];
    }
    
//----------------------------------------------------------------------------------------------------------------------

    const string MESHSYNC_PROJECT_SETTINGS_PATH = "ProjectSettings/MeshSyncSettings.asset";

    [SerializeField] private MeshSyncPlayerConfig[] m_defaultPlayerConfigs;

}

} //end namespace