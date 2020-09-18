using System.IO;

namespace Unity.MeshSync.Editor {

internal class MeshSyncEditorConstants {

    //Project settings
    internal const int UNINITIALIZED_TAB    = -1;
    internal const int GENERAL_SETTINGS_TAB             = 0;
    internal const int SCENE_CACHE_PLAYER_SETTINGS_TAB  = 1;
    internal const int MAX_SETTINGS_TAB                 = 2;
    

    //
    internal const string DCC_INSTALL_SCRIPTS_PATH = "Packages/com.unity.meshsync/Editor/DCCInstallScripts";
    internal const string PROJECT_SETTINGS_UIELEMENTS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/ProjectSettings";

    //UIElements Main
    internal const string MAIN_USER_SETTINGS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/UserSettings/UserSettings_Main";
    
    //UIElements Tabs
    internal static readonly string SERVER_SETTINGS_TAB_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "ServerSettings_Tab");
    internal static readonly string SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "SceneCachePlayerSettings_Tab");
    
    
    internal static readonly string MESHSYNC_PLAYER_CONFIG_CONTAINER_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "MeshSyncPlayerConfig_Container");
    internal static readonly string PROJECT_SETTINGS_FIELD_TEMPLATE_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "ProjectSettingsFieldTemplate");

    
    
    //UIElement Styles
    internal const string USER_SETTINGS_STYLE_PATH = "Packages/com.unity.meshsync/Editor/UIElements/UserSettings/UserSettings_Style";
    
    
    //
    internal const string DCC_PLUGINS_GITHUB_RELEASE_URL = "https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases/download/";

    
}    
    
} //end namespace