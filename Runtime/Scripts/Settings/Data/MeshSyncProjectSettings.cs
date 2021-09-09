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
        m_instance.SaveSettings();
#endif
        return m_instance;
        
    }

    
//----------------------------------------------------------------------------------------------------------------------

    //Constructor
    private MeshSyncProjectSettings() {
        ValidatePlayerConfigs();
        
    }
   
//----------------------------------------------------------------------------------------------------------------------
    protected override object GetLock() { return m_instanceLock; }
    internal override string GetSettingsPath() { return MESHSYNC_RUNTIME_SETTINGS_PATH;}

    internal ushort GetDefaultServerPort() { return m_defaultServerPort;}
    internal void SetDefaultServerPort(ushort port) { m_defaultServerPort = port;}

    internal string GetSceneCacheOutputPath()            { return m_sceneCacheOutputPath;}
    internal void   SetSceneCacheOutputPath(string path) { m_sceneCacheOutputPath = path;}
    
    internal bool   GetServerPublicAccess()            { return m_serverPublicAccess; }
    internal void   SetServerPublicAccess(bool access) { m_serverPublicAccess = access;}
    
    
    internal MeshSyncPlayerConfig   GetDefaultMeshSyncPlayerConfig() { return m_defaultMeshSyncPlayerConfig; }
    internal SceneCachePlayerConfig GetDefaultSceneCachePlayerConfig() { return m_defaultSceneCachePlayerConfig; }
    
//----------------------------------------------------------------------------------------------------------------------
    private void ValidatePlayerConfigs() {

        if (null == m_defaultMeshSyncPlayerConfig) {
            m_defaultMeshSyncPlayerConfig = new MeshSyncPlayerConfig();            
        }

        if (null == m_defaultSceneCachePlayerConfig) {
            m_defaultSceneCachePlayerConfig = new SceneCachePlayerConfig() {
                UpdateMeshColliders    = false,
                FindMaterialFromAssets = false,
                ProgressiveDisplay     = false,
            };            
        }         
    }

//----------------------------------------------------------------------------------------------------------------------
    private void UpgradeVersionToLatest() {
        m_meshSyncRuntimeSettingsVersion = ClassVersion;
        if (m_meshSyncRuntimeSettingsVersion == LATEST_VERSION) {
            return;            
        }

        if (m_meshSyncRuntimeSettingsVersion < (int) Version.SEPARATE_SCENE_CACHE_PLAYER_CONFIG) {
            if (null!= m_defaultPlayerConfigs && m_defaultPlayerConfigs.Length >= 2) {
                m_defaultMeshSyncPlayerConfig   = m_defaultPlayerConfigs[0];
                m_defaultSceneCachePlayerConfig = new SceneCachePlayerConfig(m_defaultPlayerConfigs[1]);
            }
        }

        m_defaultPlayerConfigs = null;
        m_meshSyncRuntimeSettingsVersion = ClassVersion = LATEST_VERSION;
        SaveSettings();
    }
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] private ushort m_defaultServerPort  = MeshSyncConstants.DEFAULT_SERVER_PORT;
    [SerializeField] private bool   m_serverPublicAccess = false;
    
    //Ex: "Assets/Foo"
    [SerializeField] private string m_sceneCacheOutputPath = MeshSyncConstants.DEFAULT_SCENE_CACHE_OUTPUT_PATH;
    
    
    [SerializeField] private MeshSyncPlayerConfig   m_defaultMeshSyncPlayerConfig   = null;
    [SerializeField] private SceneCachePlayerConfig m_defaultSceneCachePlayerConfig = null;
    

    [SerializeField] private int m_meshSyncRuntimeSettingsVersion = LATEST_VERSION;
    
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