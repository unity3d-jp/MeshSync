using System.Collections.Generic;
using System.IO;
using NUnit.Framework;
using Unity.AnimeToolbox.Editor;
using UnityEditor;
using UnityEngine.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;

namespace Unity.MeshSync.Editor.Tests {
internal class ProjectSettingsTest {
    
   
   
//----------------------------------------------------------------------------------------------------------------------    
    [Test]
    public void CreateRuntimeSettings() {

        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        Assert.NotNull(runtimeSettings);
        Assert.True(File.Exists(runtimeSettings.GetSettingsPath()));           
    }    


//----------------------------------------------------------------------------------------------------------------------
    [Test]
    public void CheckProjectSettingUIElements() {

        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.MAIN_PROJECT_SETTINGS_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SERVER_SETTINGS_TAB_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.TAB_BUTTON_TEMPLATE_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.PROJECT_SETTINGS_FIELD_TEMPLATE_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.MESHSYNC_PLAYER_CONFIG_CONTAINER_PATH));

        Assert.IsNotNull(LoadStyleSheet(Constants.PROJECT_SETTINGS_STYLE_PATH));
        
    }

//----------------------------------------------------------------------------------------------------------------------
 
    StyleSheet LoadStyleSheet( string path) {
        const string STYLE_EXT = ".uss";
        return AssetDatabase.LoadAssetAtPath<StyleSheet>(path + STYLE_EXT);
    }    
    
    
}

} //end namespace
