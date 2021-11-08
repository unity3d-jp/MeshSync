using System;
using System.IO;
using Unity.FilmInternalUtilities;
using UnityEngine;


namespace Unity.MeshSync {

[Serializable]
internal class MeshSyncProjectSettings : BaseJsonSettings {

    internal static MeshSyncProjectSettings GetOrCreateSettings() {
        
        if (null != m_instance) {
            return m_instance;
        }

        lock (m_instanceLock) {           
        
#if UNITY_EDITOR
            const string PATH = MESHSYNC_RUNTIME_SETTINGS_PATH;
            if (File.Exists(PATH)) {
                m_instance = FileUtility.DeserializeFromJson<MeshSyncProjectSettings>(PATH);
                m_instance.UpgradeVersionToLatest();
                m_instance.ValidatePlayerConfigs();
            }
            if (null != m_instance) {
                return m_instance;
            }            
#endif
            
            m_instance = new MeshSyncProjectSettings();
        }        

#if UNITY_EDITOR
        m_instance.Save();
#endif
        return m_instance;
        
    }

    
//----------------------------------------------------------------------------------------------------------------------

    //Constructor
    private MeshSyncProjectSettings() : base(MESHSYNC_RUNTIME_SETTINGS_PATH) {
        ValidatePlayerConfigs();
        
    }
   
//----------------------------------------------------------------------------------------------------------------------
    protected override object GetLockV() { return m_instanceLock; }
    
    protected override void OnDeserializeV() {}

    
    internal ushort GetDefaultServerPort() { return m_defaultServerPort;}
    internal void SetDefaultServerPort(ushort port) { m_defaultServerPort = port;}

    internal string GetSceneCacheOutputPath()            { return m_sceneCacheOutputPath;}
    internal void   SetSceneCacheOutputPath(string path) { m_sceneCacheOutputPath = path;}
    
    internal bool   GetServerPublicAccess()            { return m_serverPublicAccess; }
    internal void   SetServerPublicAccess(bool access) { m_serverPublicAccess = access;}
    
    
    internal MeshSyncServerConfig   GetDefaultServerConfig() { return m_defaultServerConfig; }
    internal SceneCachePlayerConfig GetDefaultSceneCachePlayerConfig() { return m_defaultSceneCachePlayerConfig; }

    internal void ResetDefaultServerConfig() {
        m_defaultServerConfig = new MeshSyncServerConfig();
        m_defaultServerConfig.SetModelImporterSettings(new ModelImporterSettings());
    }

    internal void ResetDefaultSceneCachePlayerConfig() {
        m_defaultSceneCachePlayerConfig = new SceneCachePlayerConfig() {
            UpdateMeshColliders = false,
            ProgressiveDisplay  = false,
        };            
        m_defaultSceneCachePlayerConfig.SetModelImporterSettings(new ModelImporterSettings());
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    private void ValidatePlayerConfigs() {

        if (null == m_defaultServerConfig) {
            ResetDefaultServerConfig();
        }

        if (null == m_defaultSceneCachePlayerConfig) {
            ResetDefaultSceneCachePlayerConfig();
        }         
    }

//----------------------------------------------------------------------------------------------------------------------
    private void UpgradeVersionToLatest() {
        m_meshSyncProjectSettingsVersion = ClassVersion;
        if (m_meshSyncProjectSettingsVersion == LATEST_VERSION) {
            return;            
        }

        if (m_meshSyncProjectSettingsVersion < (int) Version.SEPARATE_SCENE_CACHE_PLAYER_CONFIG) {
            if (null!= m_defaultPlayerConfigs && m_defaultPlayerConfigs.Length >= 2) {
                m_defaultServerConfig   = m_defaultPlayerConfigs[0] as MeshSyncServerConfig;
                m_defaultSceneCachePlayerConfig = new SceneCachePlayerConfig(m_defaultPlayerConfigs[1]);
            }
        }

        m_defaultPlayerConfigs = null;
        m_meshSyncProjectSettingsVersion = ClassVersion = LATEST_VERSION;
        Save();
    }
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private ushort m_defaultServerPort  = MeshSyncConstants.DEFAULT_SERVER_PORT;
    [SerializeField] private bool   m_serverPublicAccess = false;
    
    //Ex: "Assets/Foo"
    [SerializeField] private string m_sceneCacheOutputPath = MeshSyncConstants.DEFAULT_SCENE_CACHE_OUTPUT_PATH;
    
    
    [SerializeField] private MeshSyncServerConfig   m_defaultServerConfig   = null;
    [SerializeField] private SceneCachePlayerConfig m_defaultSceneCachePlayerConfig = null;
    

    [SerializeField] private int m_meshSyncProjectSettingsVersion = LATEST_VERSION;
    
    //[TODO-sin: 2021-9-9] Remove these 2 fields. Obsolete starting from 0.9.x. FormerSerializedAs doesn't work with json
    [SerializeField] private MeshSyncPlayerConfig[] m_defaultPlayerConfigs;
    [SerializeField] private int ClassVersion = LATEST_VERSION;    
    
//----------------------------------------------------------------------------------------------------------------------

    private static MeshSyncProjectSettings m_instance = null;
    private static readonly object m_instanceLock = new object();

    private const string MESHSYNC_RUNTIME_SETTINGS_PATH = "ProjectSettings/MeshSyncSettings.asset";



    private const int LATEST_VERSION = (int) Version.SEPARATE_SCENE_CACHE_PLAYER_CONFIG; 
    enum Version {
        LEGACY = 3,
        SEPARATE_SCENE_CACHE_PLAYER_CONFIG = 4,
    };


}

} //end namespace