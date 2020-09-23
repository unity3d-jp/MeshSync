using System.IO;

namespace Unity.MeshSync.Editor {

internal class MeshSyncEditorConstants {

    internal const string PACKAGE_NAME = MeshSyncConstants.PACKAGE_NAME;
    

    //Project settings
    internal const int UNINITIALIZED_TAB    = -1;
    internal const int SERVER_SETTINGS_TAB              = 0;
    internal const int SCENE_CACHE_PLAYER_SETTINGS_TAB  = 1;
    internal const int MAX_SETTINGS_TAB                 = 2;
    

    //
    internal const string DCC_INSTALL_SCRIPTS_PATH = "Packages/com.unity.meshsync/Editor/DCCInstallScripts";

    //UIElements Main
    
    //Project Settings UIElements
    internal static readonly string MAIN_PROJECT_SETTINGS_PATH            = ProjSettingsUIPath("ProjectSettings_Main");
    internal static readonly string SERVER_SETTINGS_TAB_PATH              = ProjSettingsUIPath("ServerSettings_Tab");
    internal static readonly string SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH  = ProjSettingsUIPath("SceneCachePlayerSettings_Tab");
    internal static readonly string TAB_BUTTON_TEMPLATE_PATH              = ProjSettingsUIPath("TabButtonTemplate");
    internal static readonly string PROJECT_SETTINGS_FIELD_TEMPLATE_PATH  = ProjSettingsUIPath("ProjectSettingsFieldTemplate");
    internal static readonly string MESHSYNC_PLAYER_CONFIG_CONTAINER_PATH = ProjSettingsUIPath("MeshSyncPlayerConfig_Container");

    internal static readonly string PROJECT_SETTINGS_STYLE_PATH           = ProjSettingsUIPath("ProjectSettings_Style");
    
    
    //User Settings UIElements
    internal static readonly string MAIN_USER_SETTINGS_PATH           = UserSettingsUIPath("UserSettings_Main");
    internal static readonly string DCC_TOOLS_SETTINGS_CONTAINER_PATH = UserSettingsUIPath("DCCToolsSettings_Container");
    internal static readonly string DCC_TOOL_INFO_TEMPLATE_PATH       = UserSettingsUIPath("DCCToolInfoTemplate");
    
    internal static readonly string USER_SETTINGS_STYLE_PATH = UserSettingsUIPath("UserSettings_Style");
    
    //
    internal const string DCC_PLUGINS_GITHUB_RELEASE_URL = "https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases/download/";


    
    //Private
    private const string PROJECT_SETTINGS_UIELEMENTS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/ProjectSettings";
    private const string USER_SETTINGS_UIELEMENTS_PATH    = "Packages/com.unity.meshsync/Editor/UIElements/UserSettings";
    
    
//----------------------------------------------------------------------------------------------------------------------
    private static string ProjSettingsUIPath(string uiElementRelativePath) {
        return Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, uiElementRelativePath);
    }

    private static string UserSettingsUIPath(string uiElementRelativePath) {
        return Path.Combine(MeshSyncEditorConstants.USER_SETTINGS_UIELEMENTS_PATH, uiElementRelativePath);

    }
    
    
}    
    
} //end namespace