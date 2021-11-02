using System.Collections.Generic;
using System.IO;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;

namespace Unity.MeshSync.Editor {

internal static class MeshSyncEditorConstants {
    
    [InitializeOnLoadMethod]
    static void MeshSyncEditorConstants_OnLoad() {
        bool parsed = PackageVersion.TryParse(Lib.GetPluginVersion(), out m_pluginVersion);
        Assert.IsTrue(parsed);        
    }

    internal static PackageVersion GetPluginVersion() {
        return m_pluginVersion;
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    internal static readonly string[] ANIMATION_INTERPOLATION_ENUMS = System.Enum.GetNames( typeof( InterpolationMode ) );
    internal static readonly string[] Z_UP_CORRECTION_ENUMS         = System.Enum.GetNames( typeof( ZUpCorrectionMode ) );
    
    //
    internal const string SCENE_CACHE_IMPORTER_SETTINGS_PROP = "m_importerSettings";
    internal const string MODEL_IMPORTER_CREATE_MATERIALS_PROP = "CreateMaterials";
    internal const string MODEL_IMPORTER_MATERIAL_SEARCH_MODE_PROP = "MaterialSearchMode";
    
    //Project settings
    internal const int            UNINITIALIZED_TAB               = -1;
    internal const int            SERVER_SETTINGS_TAB             = 0;
    internal const int            SCENE_CACHE_PLAYER_SETTINGS_TAB = 1;
    internal const int            MAX_SETTINGS_TAB                = 2;

    //
    internal const string DCC_INSTALL_SCRIPTS_PATH = "Packages/com.unity.meshsync/Editor/DCCInstallScripts";
    
    //key: default folder name
    internal static readonly Dictionary<string, DCCToolInfo> SUPPORTED_DCC_TOOLS_BY_FOLDER = new Dictionary<string, DCCToolInfo>() {
        { "maya2018", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2018" ) },
        { "maya2019", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2019" ) },
        { "maya2020", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2020" ) },
        { "maya2022", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2022" ) },
        { "3ds Max 2018", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2018" ) },
        { "3ds Max 2019", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2019" ) },
        { "3ds Max 2020", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2020" ) },
        { "3ds Max 2021", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2021" ) },

#if UNITY_EDITOR_WIN        
        { "Blender 2.83", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },
        { "Blender 2.90", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
        { "Blender 2.91", new DCCToolInfo(DCCToolType.BLENDER, "2.91" ) },
        { "Blender 2.92", new DCCToolInfo(DCCToolType.BLENDER, "2.92" ) },
        { "Blender 2.93", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
#elif UNITY_EDITOR_OSX        
        { "Blender.app", new DCCToolInfo(DCCToolType.BLENDER, null ) },
#elif UNITY_EDITOR_LINUX
        { "blender-2.83.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.1-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.2-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.3-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.4-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.5-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },        
        { "blender-2.83.6-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },
        { "blender-2.83.7-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },
        { "blender-2.83.8-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },
        { "blender-2.83.9-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.83" ) },
        { "blender-2.83.10-linux64", new DCCToolInfo(DCCToolType.BLENDER,"2.83" ) },
        { "blender-2.83.12-linux64", new DCCToolInfo(DCCToolType.BLENDER,"2.83" ) },
        { "blender-2.83.13-linux64", new DCCToolInfo(DCCToolType.BLENDER,"2.83" ) },
        { "blender-2.83.14-stable+blender-v2-83-release.759fd9e4c204-linux.x86_64-release", new DCCToolInfo(DCCToolType.BLENDER,"2.83" ) },
        { "blender-2.83.15-stable+blender-v283-release.fd3036520101-linux.x86_64-release", new DCCToolInfo(DCCToolType.BLENDER,"2.83" ) },
        { "blender-2.83.16-linux-x64", new DCCToolInfo(DCCToolType.BLENDER,"2.83" ) },
        { "blender-2.90.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
        { "blender-2.90.1-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
        { "blender-2.91.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.91" ) },
        { "blender-2.91.2-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.91" ) },
        { "blender-2.92.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.92" ) },
        { "blender-2.93.0-stable+blender-v293-release.84da05a8b806-linux.x86_64-release", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.1-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        
#endif        
        
    };
    

    //UIElements Main
    
    //Project Settings UIElements
    internal static readonly string MAIN_PROJECT_SETTINGS_PATH           = ProjSettingsUIPath("ProjectSettings_Main");
    internal static readonly string SERVER_SETTINGS_TAB_PATH             = ProjSettingsUIPath("ServerSettings_Tab");
    internal static readonly string SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH = ProjSettingsUIPath("SceneCachePlayerSettings_Tab");
    internal static readonly string TAB_BUTTON_TEMPLATE_PATH             = ProjSettingsUIPath("TabButtonTemplate");
    internal static readonly string PROJECT_SETTINGS_FIELD_TEMPLATE_PATH = ProjSettingsUIPath("ProjectSettingsFieldTemplate");
    internal static readonly string PROJECT_SETTINGS_STYLE_PATH          = ProjSettingsUIPath("ProjectSettings_Style");

    //Project Settings UIElements - Config    
    internal static readonly string SERVER_CONFIG_CONTAINER_PATH             = ProjSettingsUIPath("ServerConfig_Container");
    internal static readonly string SCENE_CACHE_PLAYER_CONFIG_CONTAINER_PATH = ProjSettingsUIPath("SceneCachePlayerConfig_Container");
    
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
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    private static PackageVersion m_pluginVersion;
    
    
    
}    
    
} //end namespace