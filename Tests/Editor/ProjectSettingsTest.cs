using System.Collections.Generic;
using System.IO;
using NUnit.Framework;
using Unity.AnimeToolbox.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.TestTools;
using UnityEngine.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;

namespace Unity.MeshSync.Editor.Tests {
internal class ProjectSettingsTest {
    
    
    [Test]
    public void CheckInstalledDCCTools() {

        Dictionary<string, DCCToolInfo> dccToolInfos = DCCFinderUtility.FindInstalledDCCTools();
        Assert.GreaterOrEqual(dccToolInfos.Count,0);
    }    
    
//----------------------------------------------------------------------------------------------------------------------    
    [Test]
    public void CreateSettings() {
        MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
        Assert.NotNull(settings);
        Assert.True(File.Exists(MeshSyncEditorSettings.GetSettingsPath()));

        MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        Assert.NotNull(runtimeSettings);
        Assert.True(File.Exists(runtimeSettings.GetSettingsPath()));
            
    }    

    
//----------------------------------------------------------------------------------------------------------------------    
    
    [Test]
    [UnityPlatform(RuntimePlatform.OSXEditor)]
    public void FindDCCToolVersionsOnOSX() {
        string version = null;
        version = DCCFinderUtility.FindMayaVersion("/Applications/Maya 2019/Maya.app/Contents/MacOS/Maya");
        Assert.AreEqual("2019", version);

        version = DCCFinderUtility.FindMayaVersion("/Applications/Maya2019/Maya.app/Contents/MacOS/Maya");
        Assert.AreEqual("2019", version);
    }    

    [Test]
    [UnityPlatform(RuntimePlatform.WindowsEditor)]
    public void FindDCCToolVersionsOnWindows() {

        string version = null;
        version = DCCFinderUtility.FindMayaVersion(@"C:\Program Files\Autodesk\maya2019\bin\maya.exe");
        Assert.AreEqual("2019", version);

        version = DCCFinderUtility.FindMayaVersion(@"C:\Program Files\Autodesk\maya2020\bin\maya.exe");
        Assert.AreEqual("2020", version);

        version = DCCFinderUtility.Find3DSMaxVersion(@"C:\Program Files\Autodesk\3ds Max 2019\3dsmax.exe");
        Assert.AreEqual("2019", version);

        version = DCCFinderUtility.Find3DSMaxVersion(@"C:\Program Files\Autodesk\3ds Max 2020\3dsmax.exe");
        Assert.AreEqual("2020", version);

    }    

//----------------------------------------------------------------------------------------------------------------------    
    [Test]
    public void CheckInstallInfoPath() {
        string mayaInfoPath = DCCPluginInstallInfo.GetInstallInfoPath(
            new DCCToolInfo(DCCToolType.AUTODESK_MAYA, "2019")
        );
        Assert.NotNull(mayaInfoPath);
        
        string _3dsMaxInfoPath = DCCPluginInstallInfo.GetInstallInfoPath(
            new DCCToolInfo(DCCToolType.AUTODESK_3DSMAX, "2019")
        );
        Assert.NotNull(_3dsMaxInfoPath);
        
        Assert.AreNotEqual(mayaInfoPath,_3dsMaxInfoPath);
        
    }    

//----------------------------------------------------------------------------------------------------------------------
    [Test]
    public void CheckProjectSettingUIElements() {

        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.MAIN_PROJECT_SETTINGS_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SERVER_SETTINGS_TAB_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.TAB_BUTTON_TEMPLATE_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.PROJECT_SETTINGS_STYLE_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.PROJECT_SETTINGS_FIELD_TEMPLATE_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.MESHSYNC_PLAYER_CONFIG_CONTAINER_PATH));
        
    }

    [Test]
    public void CheckUserSettingUIElements() {
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.MAIN_USER_SETTINGS_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.USER_SETTINGS_STYLE_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.DCC_TOOLS_SETTINGS_CONTAINER_PATH));
        Assert.IsNotNull(UIElementsEditorUtility.LoadVisualTreeAsset(Constants.DCC_TOOL_INFO_TEMPLATE_PATH));        
    }

//----------------------------------------------------------------------------------------------------------------------
    
    StyleSheet LoadStyleSheet( string path) {
        const string STYLE_EXT = ".uss";
        return AssetDatabase.LoadAssetAtPath<StyleSheet>(path + STYLE_EXT);
    }
    
    
    
}

} //end namespace
