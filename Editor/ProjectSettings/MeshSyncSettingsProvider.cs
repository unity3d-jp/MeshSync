using System.Collections.Generic;
using System.IO;
using Unity.AnimeToolbox;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;

namespace UnityEditor.MeshSync {
	
// Create a new type of Settings Asset.
class MyCustomSettings : ScriptableObject
{
    public const string k_MyCustomSettingsPath = "Assets/Editor/MyCustomSettings.asset";

    [SerializeField]
    private int m_Number;

    [SerializeField]
    private string m_SomeString;

    internal static MyCustomSettings GetOrCreateSettings()
    {
        var settings = AssetDatabase.LoadAssetAtPath<MyCustomSettings>(k_MyCustomSettingsPath);
        if (settings == null)
        {
            settings = ScriptableObject.CreateInstance<MyCustomSettings>();
            settings.m_Number = 42;
            settings.m_SomeString = "The answer to the universe";
            Directory.CreateDirectory(k_MyCustomSettingsPath);
            AssetDatabase.CreateAsset(settings, k_MyCustomSettingsPath);
            AssetDatabase.SaveAssets();
        }
        return settings;
    }

    internal static SerializedObject GetSerializedSettings()
    {
        return new SerializedObject(GetOrCreateSettings());
    }
}


class MeshSyncSettingsProvider : SettingsProvider {

	
	
	MeshSyncSettingsProvider() : base(PROJECT_SETTINGS_MENU_PATH,SettingsScope.Project) {
		m_tabs = new IMeshSyncSettingsTab[MeshSyncEditorConstants.MAX_SETTINGS_TAB];
		m_tabs[MeshSyncEditorConstants.GENERAL_SETTINGS_TAB] = new GeneralSettingsTab();
		m_tabs[MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_TAB] = new DCCToolsSettingsTab();
		

		//activateHandler is called when the user clicks on the Settings item in the Settings window.
		activateHandler = (string searchContext, VisualElement rootElement) => {

			var rootElement1 = rootElement;
			
			//Main Tree
			VisualTreeAsset main = UIElementsEditorUtility.LoadVisualTreeAsset(
				Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "ProjectSettings_Main")
			);
			main.CloneTree(rootElement1);


			//Buttons
			VisualElement toolbarContainer = rootElement1.Query<VisualElement>("ToolbarContainer");
			VisualTreeAsset toolbarButtonTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
				Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH, "ToolbarButtonTemplate")
			);
			
			
			toolbarContainer.Add(CreateButton(toolbarButtonTemplate, "General Settings", OnGeneralSettingsTabMouseDown));
			toolbarContainer.Add(CreateButton(toolbarButtonTemplate, "DCC Tools", OnDCCToolsTabMouseDown));
			

			//Style
			UIElementsEditorUtility.LoadAndAddStyle(
				rootElement1.styleSheets,
				Path.Combine(PROJECT_SETTINGS_UIELEMENTS_PATH,"ProjectSettings_Style")
			);

			m_content = rootElement1.Query<VisualElement>("Content");
			SetupTab(MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_TAB);


			// var settings = MyCustomSettings.GetSerializedSettings();
			//
			// // rootElement is a VisualElement. If you add any children to it, the OnGUI function
			// // isn't called because the SettingsProvider uses the UIElements drawing framework.
			// //var styleSheet = AssetDatabase.LoadAssetAtPath<StyleSheet>("Assets/Editor/settings_ui.uss");
			// //rootElement.styleSheets.Add(styleSheet);
			// var title = new Label() {
			// 	text = "Custom UI Elements"
			// };
			// title.AddToClassList("title");
			// title.RegisterCallback<MouseDownEvent>(OnMouseDownEvent);
			//
			// rootElement.Add(title);
			//
			// var properties = new VisualElement() {
			// 	style = {
			// 		flexDirection = FlexDirection.Column
			// 	}
			// };
			// properties.AddToClassList("property-list");
			// rootElement.Add(properties);
			//
			// var tf = new TextField() {
			// 	value = settings.FindProperty("m_SomeString").stringValue
			// };
			// tf.AddToClassList("property-value");
			// properties.Add(tf);
		};

		// Populate the search keywords to enable smart search filtering and label highlighting:
		keywords = new HashSet<string>(new[] {"Number", "Some String"});
		
	}
	
//----------------------------------------------------------------------------------------------------------------------	

	private int GetSelectedTab() { return m_selectedTab; } 

//----------------------------------------------------------------------------------------------------------------------	
	private TemplateContainer CreateButton(VisualTreeAsset template, string labelText, 
		EventCallback<MouseEventBase<MouseDownEvent>> mouseDownEvent) 
	{
		TemplateContainer button = template.CloneTree();
		Label buttonLabel = button.Query<Label>().First();
		buttonLabel.text = labelText;
		buttonLabel.RegisterCallback<MouseDownEvent>(mouseDownEvent);
		return button;
	}
	
//----------------------------------------------------------------------------------------------------------------------	

	#region Button Events
	
	static void OnGeneralSettingsTabMouseDown(MouseEventBase<MouseDownEvent> evt) {
		m_settingsProvider.SetupTab(MeshSyncEditorConstants.GENERAL_SETTINGS_TAB);
	}

	static void OnDCCToolsTabMouseDown(MouseEventBase<MouseDownEvent> evt) {
		m_settingsProvider.SetupTab(MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_TAB);
	}
	#endregion	

//----------------------------------------------------------------------------------------------------------------------
	
	private void SetupTab(int tab) {
		if (tab == m_selectedTab)
			return;
		
		m_selectedTab = tab;
		
		m_content.Clear();
		m_tabs[m_selectedTab].Setup(m_content);
		
	}
	
	
//----------------------------------------------------------------------------------------------------------------------

    [SettingsProvider]
    public static SettingsProvider CreateMeshSyncSettingsProvider() {
	    m_settingsProvider = new MeshSyncSettingsProvider();
	    return m_settingsProvider;
    }
    
//----------------------------------------------------------------------------------------------------------------------

	private int m_selectedTab = MeshSyncEditorConstants.UNINITIALIZED_TAB;
	private IMeshSyncSettingsTab[] m_tabs = null;
	
	private VisualElement m_content = null;

	private static MeshSyncSettingsProvider m_settingsProvider = null;

	private const string PROJECT_SETTINGS_MENU_PATH = "Project/MeshSync";
	private const string PROJECT_SETTINGS_UIELEMENTS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/ProjectSettings";
}

//----------------------------------------------------------------------------------------------------------------------
	
	// 	class Styles
	// 	{
	// 		public static GUIContent number = new GUIContent("My Number");
	// 		public static GUIContent someString = new GUIContent("Some string");
	// 	}
	// 	
	// 	// Register the SettingsProvider
	// 	[SettingsProvider]
	// 	public static SettingsProvider CreateAssetGraphSettingsProvider() {
	// 		var provider = new AssetGraphSettingsProvider("Project/MeshSync", SettingsScope.Project);
	// 		
	// 		// Automatically extract all keywords from the Styles.
	// 		provider.keywords = GetSearchKeywordsFromGUIContentProperties<Styles>();			
	//
	// 		return provider;
	// 	}
	// }
}
