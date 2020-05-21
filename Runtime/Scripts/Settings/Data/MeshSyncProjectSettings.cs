using System;
using System.IO;
using Unity.AnimeToolbox;
using UnityEngine;


namespace Unity.MeshSync {

[Serializable]
internal class MeshSyncProjectSettings : BaseJsonSettings {

    internal static MeshSyncProjectSettings GetInstance() {
        
        if (null != m_instance) {
            return m_instance;
        }

        const string PATH = MESHSYNC_PROJECT_SETTINGS_PATH;
        lock (m_instanceLock) {           
        
#if UNITY_EDITOR
            if (File.Exists(PATH)) {
                m_instance = FileUtility.DeserializeFromJson<MeshSyncProjectSettings>(PATH);                
            }
            if (null != m_instance) {
                return m_instance;
            }
#endif
            
            m_instance = new MeshSyncProjectSettings();
        }        

#if UNITY_EDITOR
        m_instance.SaveSettings();
#endif
        return m_instance;
        
    }

    
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
    protected override object GetLock() { return m_instanceLock; }
    public override string GetSettingsPath() { return MESHSYNC_PROJECT_SETTINGS_PATH;}

//----------------------------------------------------------------------------------------------------------------------

    internal MeshSyncPlayerConfig GetDefaultPlayerConfig(MeshSyncObjectType objectType) {
        return m_defaultPlayerConfigs[(int) objectType];
    }

    
//----------------------------------------------------------------------------------------------------------------------

    private static MeshSyncProjectSettings m_instance = null;
    private static readonly object m_instanceLock = new object();

    private const string MESHSYNC_PROJECT_SETTINGS_PATH = "ProjectSettings/MeshSyncSettings.asset";

    [SerializeField] private MeshSyncPlayerConfig[] m_defaultPlayerConfigs;

}

} //end namespace