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
        
        m_defaultPlayerConfigs = new MeshSyncPlayerConfig[(int) MeshSyncPlayerType.NUM_TYPES]; 

        MeshSyncPlayerConfig config = new MeshSyncPlayerConfig();
        m_defaultPlayerConfigs[(int) MeshSyncPlayerType.SERVER] = config;

        config = new MeshSyncPlayerConfig {
            UpdateMeshColliders = false, 
            FindMaterialFromAssets = false
        };
        m_defaultPlayerConfigs[(int) MeshSyncPlayerType.CACHE_PLAYER] = config;
    }
   
//----------------------------------------------------------------------------------------------------------------------
    protected override object GetLock() { return m_instanceLock; }
    public override string GetSettingsPath() { return MESHSYNC_PROJECT_SETTINGS_PATH;}

//----------------------------------------------------------------------------------------------------------------------

    internal MeshSyncPlayerConfig GetDefaultPlayerConfig(MeshSyncPlayerType playerType) {
        return m_defaultPlayerConfigs[(int) playerType];
    }

    
//----------------------------------------------------------------------------------------------------------------------

    private static MeshSyncProjectSettings m_instance = null;
    private static readonly object m_instanceLock = new object();

    private const string MESHSYNC_PROJECT_SETTINGS_PATH = "ProjectSettings/MeshSyncSettings.asset";

    [SerializeField] private MeshSyncPlayerConfig[] m_defaultPlayerConfigs;

}

} //end namespace