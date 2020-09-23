using System.IO;

namespace Unity.MeshSync.Editor {

internal class MeshSyncEditorConstants {

    //Project settings
    internal const int UNINITIALIZED_TAB    = -1;
    internal const int SERVER_SETTINGS_TAB              = 0;
    internal const int SCENE_CACHE_PLAYER_SETTINGS_TAB  = 1;
    internal const int MAX_SETTINGS_TAB                 = 2;
    

    //
    internal const string DCC_INSTALL_SCRIPTS_PATH = "Packages/com.unity.meshsync/Editor/DCCInstallScripts";

    //UIElements Main
    internal const string MAIN_PROJECT_SETTINGS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/ProjectSettings/ProjectSettings_Main";
    internal const string MAIN_USER_SETTINGS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/UserSettings/UserSettings_Main";
    
    //Project Settings UIElements
    internal static readonly string SERVER_SETTINGS_TAB_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "ServerSettings_Tab");
    internal static readonly string SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "SceneCachePlayerSettings_Tab");

    internal static readonly string TAB_BUTTON_TEMPLATE_PATH = ProjectSettingsUIPath("TabButtonTemplate");
    internal static readonly string PROJECT_SETTINGS_STYLE_PATH = ProjectSettingsUIPath("ProjectSettings_Style");
    
    
    
    internal static readonly string MESHSYNC_PLAYER_CONFIG_CONTAINER_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "MeshSyncPlayerConfig_Container");
    internal static readonly string PROJECT_SETTINGS_FIELD_TEMPLATE_PATH =
        Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "ProjectSettingsFieldTemplate");
    
    
    //User Settings UIElements
    internal const string USER_SETTINGS_STYLE_PATH = "Packages/com.unity.meshsync/Editor/UIElements/UserSettings/UserSettings_Style";

    internal static readonly string DCC_TOOLS_SETTINGS_CONTAINER_PATH = UserSettingsUIPath("DCCToolsSettings_Container");
    internal static readonly string DCC_TOOL_INFO_TEMPLATE_PATH       = UserSettingsUIPath("DCCToolInfoTemplate");
    
    
    //
    internal const string DCC_PLUGINS_GITHUB_RELEASE_URL = "https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases/download/";


    
    //Private
    private const string PROJECT_SETTINGS_UIELEMENTS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/ProjectSettings";
    private const string USER_SETTINGS_UIELEMENTS_PATH    = "Packages/com.unity.meshsync/Editor/UIElements/UserSettings";
    
    
//----------------------------------------------------------------------------------------------------------------------
    private static string ProjectSettingsUIPath(string uiElementRelativePath) {
        return Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, uiElementRelativePath);
    }

    private static string UserSettingsUIPath(string uiElementRelativePath) {
        return Path.Combine(MeshSyncEditorConstants.USER_SETTINGS_UIELEMENTS_PATH, uiElementRelativePath);

    }
    
    
}    
    
} //end namespace