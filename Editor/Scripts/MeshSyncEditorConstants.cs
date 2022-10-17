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

        foreach (KeyValuePair<string, DCCToolInfo> kv in SUPPORTED_DCC_TOOLS_BY_FOLDER) {
            DCCToolInfo     dccToolInfo = kv.Value;
            if (!SUPPORTED_DCC_TOOLS.TryGetValue(dccToolInfo.Type, out HashSet<string> versions)) {
                versions = new HashSet<string>();
                SUPPORTED_DCC_TOOLS.Add(dccToolInfo.Type, versions);
            }
            versions.Add(dccToolInfo.DCCToolVersion);
        }
    }

    internal static PackageVersion GetPluginVersion() {
        return m_pluginVersion;
    }

//----------------------------------------------------------------------------------------------------------------------    
    
    internal static readonly string[] ANIMATION_INTERPOLATION_ENUMS = System.Enum.GetNames( typeof( InterpolationMode ) );
    internal static readonly string[] Z_UP_CORRECTION_ENUMS         = System.Enum.GetNames( typeof( ZUpCorrectionMode ) );
        
    //Project settings
    internal const int UNINITIALIZED_TAB               = -1;
    internal const int SERVER_SETTINGS_TAB             = 0;
    internal const int SCENE_CACHE_PLAYER_SETTINGS_TAB = 1;
    internal const int EDITOR_SERVER_SETTINGS_TAB      = 2;
    internal const int MAX_SETTINGS_TAB                = 3;

    //
    internal const string DCC_INSTALL_SCRIPTS_PATH = "Packages/com.unity.meshsync/Editor/DCCInstallScripts";
    
    //key: default folder name
    internal static readonly Dictionary<string, DCCToolInfo> SUPPORTED_DCC_TOOLS_BY_FOLDER = new Dictionary<string, DCCToolInfo>() {
        { "maya2018", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2018" ) },
        { "maya2019", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2019" ) },
        { "maya2020", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2020" ) },
        { "maya2022", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2022" ) },
        { "maya2023", new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2023" ) },
        { "3ds Max 2018", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2018" ) },
        { "3ds Max 2019", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2019" ) },
        { "3ds Max 2020", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2020" ) },
        { "3ds Max 2021", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2021" ) },
        { "3ds Max 2022", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2022" ) },
        { "3ds Max 2023", new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2023" ) },

#if UNITY_EDITOR_WIN
        { "Blender 2.90", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
        { "Blender 2.91", new DCCToolInfo(DCCToolType.BLENDER, "2.91" ) },
        { "Blender 2.92", new DCCToolInfo(DCCToolType.BLENDER, "2.92" ) },
        { "Blender 2.93", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "Blender 3.0", new DCCToolInfo(DCCToolType.BLENDER, "3.0" ) },
        { "Blender 3.1", new DCCToolInfo(DCCToolType.BLENDER, "3.1" ) },
        { "Blender 3.2", new DCCToolInfo(DCCToolType.BLENDER, "3.2" ) },
        { "Blender 3.3", new DCCToolInfo(DCCToolType.BLENDER, "3.3" ) },
#elif UNITY_EDITOR_OSX
        { "Blender/2.90", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) }, 
        { "Blender/2.91", new DCCToolInfo(DCCToolType.BLENDER, "2.91" ) }, 
        { "Blender/2.92", new DCCToolInfo(DCCToolType.BLENDER, "2.92" ) }, 
        { "Blender/2.93", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) }, 
        { "Blender/3.0", new DCCToolInfo(DCCToolType.BLENDER, "3.0" ) }, 
        { "Blender/3.1", new DCCToolInfo(DCCToolType.BLENDER, "3.1" ) }, 
        { "Blender/3.2", new DCCToolInfo(DCCToolType.BLENDER, "3.2" ) }, 
        { "Blender/3.3", new DCCToolInfo(DCCToolType.BLENDER, "3.3" ) }, 
        { "Blender.app", new DCCToolInfo(DCCToolType.BLENDER, null ) },  //app directly
#elif UNITY_EDITOR_LINUX
        { "blender-2.90.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
        { "blender-2.90.1-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.90" ) },
        { "blender-2.91.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.91" ) },
        { "blender-2.91.2-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.91" ) },
        { "blender-2.92.0-linux64", new DCCToolInfo(DCCToolType.BLENDER, "2.92" ) },
        { "blender-2.93.0-stable+blender-v293-release.84da05a8b806-linux.x86_64-release", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.1-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.2-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.3-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.4-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.5-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.6-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.7-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.8-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.9-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-2.93.10-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "2.93" ) },
        { "blender-3.0.1-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.0" ) },
        { "blender-3.1.0-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.1" ) },
        { "blender-3.1.1-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.1" ) },
        { "blender-3.1.2-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.1" ) },
        { "blender-3.2.0-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.2" ) },
        { "blender-3.2.1-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.2" ) },
        { "blender-3.2.2-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.2" ) },
        { "blender-3.3.0-linux-x64", new DCCToolInfo(DCCToolType.BLENDER, "3.3" ) },
#endif        
        
    };

    internal static readonly Dictionary<DCCToolType, HashSet<string>> SUPPORTED_DCC_TOOLS = new Dictionary<DCCToolType, HashSet<string>>();                            
    

    //UIElements Main
    
    //Project Settings UIElements
    internal static readonly string MAIN_PROJECT_SETTINGS_PATH           = ProjSettingsUIPath("ProjectSettings_Main");
    internal static readonly string SERVER_SETTINGS_TAB_PATH             = ProjSettingsUIPath("ServerSettings_Tab");
    internal static readonly string EDITOR_SERVER_SETTINGS_TAB_PATH      = ProjSettingsUIPath("EditorServerSettings_Tab");
    internal static readonly string SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH = ProjSettingsUIPath("SceneCachePlayerSettings_Tab");
    internal static readonly string TAB_BUTTON_TEMPLATE_PATH             = ProjSettingsUIPath("TabButtonTemplate");
    internal static readonly string COMPONENT_SYNC_FIELDS_TEMPLATE_PATH  = ProjSettingsUIPath("ComponentSyncFieldsTemplate");    
    internal static readonly string PROJECT_SETTINGS_STYLE_PATH          = ProjSettingsUIPath("ProjectSettings_Style");

    //Project Settings UIElements - Config    
    internal static readonly string SERVER_CONFIG_CONTAINER_PATH             = ProjSettingsUIPath("ServerConfig_Container");
    internal static readonly string SCENE_CACHE_PLAYER_CONFIG_CONTAINER_PATH = ProjSettingsUIPath("SceneCachePlayerConfig_Container");
    
    //User Settings UIElements
    internal static readonly string MAIN_USER_SETTINGS_PATH           = UserSettingsUIPath("UserSettings_Main");
    internal static readonly string DCC_TOOLS_SETTINGS_CONTAINER_PATH = UserSettingsUIPath("DCCToolsSettings_Container");
    internal static readonly string DCC_TOOL_INFO_TEMPLATE_PATH       = UserSettingsUIPath("DCCToolInfoTemplate");
    
    internal static readonly string USER_SETTINGS_STYLE_PATH = UserSettingsUIPath("UserSettings_Style");
    
    internal const string SCENE_CACHE_EXTENSION = "sc";
    
    internal const string SHORTCUT_TOGGLE_KEYFRAME = "MeshSync/Toggle KeyFrame";
    
    
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