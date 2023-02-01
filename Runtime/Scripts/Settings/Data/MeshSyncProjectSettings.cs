using System;
using System.IO;
using Unity.FilmInternalUtilities;
using UnityEngine;


namespace Unity.MeshSync {
[Serializable]
[Json(MESHSYNC_RUNTIME_SETTINGS_PATH)]
internal class MeshSyncProjectSettings : BaseJsonSingleton<MeshSyncProjectSettings> {
    public MeshSyncProjectSettings() : base() {
        ValidatePlayerConfigs();
    }

    protected override int GetLatestVersionV() {
        return LATEST_VERSION;
    }

    protected override void UpgradeToLatestVersionV(int prevVersion, int curVersion) {
    }

    protected override void OnAfterDeserializeInternalV() {
    }

//----------------------------------------------------------------------------------------------------------------------

    internal ushort GetDefaultServerPort() {
        return m_defaultServerPort;
    }

    internal void SetDefaultServerPort(ushort port) {
        m_defaultServerPort = port;
    }

    internal string GetSceneCacheOutputPath() {
        return m_sceneCacheOutputPath;
    }

    internal void SetSceneCacheOutputPath(string path) {
        m_sceneCacheOutputPath = path;
    }

    internal bool GetServerPublicAccess() {
        return m_serverPublicAccess;
    }

    internal void SetServerPublicAccess(bool access) {
        m_serverPublicAccess = access;
    }

    internal MeshSyncServerConfig GetDefaultServerConfig() {
        return m_defaultServerConfig;
    }

    internal SceneCachePlayerConfig GetDefaultSceneCachePlayerConfig() {
        return m_defaultSceneCachePlayerConfig;
    }

    internal void ResetDefaultServerConfig() {
        m_defaultServerConfig = new MeshSyncServerConfig();
    }

    internal void ResetDefaultSceneCachePlayerConfig() {
        m_defaultSceneCachePlayerConfig = new SceneCachePlayerConfig() {
            UpdateMeshColliders = false,
            ProgressiveDisplay  = false
        };
    }

//----------------------------------------------------------------------------------------------------------------------
    private void ValidatePlayerConfigs() {
        if (null == m_defaultServerConfig) ResetDefaultServerConfig();

        if (null == m_defaultSceneCachePlayerConfig) ResetDefaultSceneCachePlayerConfig();
    }

//----------------------------------------------------------------------------------------------------------------------

    [SerializeField] private ushort m_defaultServerPort  = MeshSyncConstants.DEFAULT_SERVER_PORT;
    [SerializeField] private bool   m_serverPublicAccess = false;

    //Ex: "Assets/Foo"
    [SerializeField] private string m_sceneCacheOutputPath = MeshSyncConstants.DEFAULT_SCENE_CACHE_OUTPUT_PATH;


    [SerializeField] private MeshSyncServerConfig   m_defaultServerConfig           = null;
    [SerializeField] private SceneCachePlayerConfig m_defaultSceneCachePlayerConfig = null;

//----------------------------------------------------------------------------------------------------------------------

    private const string MESHSYNC_RUNTIME_SETTINGS_PATH = "ProjectSettings/MeshSyncSettings.asset";


    private const int LATEST_VERSION = (int)Version.SeparateSceneCachePlayerConfig;

    private enum Version {
        Legacy                         = 3,
        SeparateSceneCachePlayerConfig = 4
    };
}
} //end namespace