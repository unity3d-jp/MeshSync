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

	private void DoSomething(VisualElement element) {
		
	}
	
	MeshSyncSettingsProvider() : base(PROJECT_SETTINGS_MENU_PATH,SettingsScope.Project) {

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
			RefreshContent();


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

	private ProjectSettingsTab GetSelectedTab() { return m_selectedTab; } 

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
		if (ProjectSettingsTab.GENERAL_SETTINGS == m_settingsProvider.GetSelectedTab())
			return;
		
		m_settingsProvider.SetTab(ProjectSettingsTab.GENERAL_SETTINGS);
	}

	static void OnDCCToolsTabMouseDown(MouseEventBase<MouseDownEvent> evt) {
		if (ProjectSettingsTab.DCC_TOOLS == m_settingsProvider.GetSelectedTab())
			return;
		
		m_settingsProvider.SetTab(ProjectSettingsTab.DCC_TOOLS);
	}
	#endregion	

//----------------------------------------------------------------------------------------------------------------------
	
	private void SetTab(ProjectSettingsTab tab) {
		m_selectedTab = tab;
		RefreshContent();
	}
	
//----------------------------------------------------------------------------------------------------------------------
	
	private void RefreshContent() {
		m_content.Clear();
		switch (m_selectedTab) {
			case ProjectSettingsTab.GENERAL_SETTINGS:
				m_content.Add(new Label("General Settings Content"));
				break;
			case ProjectSettingsTab.DCC_TOOLS :
				m_content.Add(new Label("DCC Tools Content"));
				break;
		}	
	}
	
	
//----------------------------------------------------------------------------------------------------------------------

    [SettingsProvider]
    public static SettingsProvider CreateMeshSyncSettingsProvider() {
	    m_settingsProvider = new MeshSyncSettingsProvider();
	    return m_settingsProvider;
    }
    
//----------------------------------------------------------------------------------------------------------------------

	private ProjectSettingsTab m_selectedTab = ProjectSettingsTab.DCC_TOOLS;
	private VisualElement m_content = null;

	private static MeshSyncSettingsProvider m_settingsProvider = null;

	private const string PROJECT_SETTINGS_MENU_PATH = "Project/MeshSync";
	private const string PROJECT_SETTINGS_UIELEMENTS_PATH = "Packages/com.unity.meshsync/Editor/UIElements/ProjectSettings";
}

//----------------------------------------------------------------------------------------------------------------------

	//
	// class AssetGraphSettingsProvider : SettingsProvider
	// {
	// 	private AssetBundlesSettingsTab     m_abTab;
	// 	private ExecutionOrderSettingsTab   m_execTab;
	// 	private Mode m_mode;
	//
	// 	enum Mode : int {
	// 		AssetBundleSettings,
	// 		ExecutionOrderSettings
	// 	}
	// 	
	// 	public AssetGraphSettingsProvider(string path, SettingsScope scope = SettingsScope.Project)
	// 		: base(path, scope)
	// 	{
	// 		m_abTab = new AssetBundlesSettingsTab ();
	// 		m_execTab = new ExecutionOrderSettingsTab ();
	// 		m_mode = Mode.AssetBundleSettings;			
	// 	}
	//
	// 	public override void OnGUI(string searchContext)
	// 	{
	// 		DrawToolBar ();
	//
	// 		switch (m_mode) {
	// 			case Mode.AssetBundleSettings:
	// 				m_abTab.OnGUI ();
	// 				break;
	// 			case Mode.ExecutionOrderSettings:
	// 				m_execTab.OnGUI ();
	// 				break;
	// 		}
	
	
	
	
	
	
	// 	}
	// 	
	// 	
	// 	
	// 	private void DrawToolBar() {
	// 		GUILayout.BeginHorizontal();
	// 		GUILayout.FlexibleSpace();
	// 		float toolbarWidth = 300f;
	// 		string[] labels = new string[] { "Asset Bundles", "Execution Order" };
	// 		m_mode = (Mode)GUILayout.Toolbar((int)m_mode, labels, "LargeButton", GUILayout.Width(toolbarWidth) );
	// 		GUILayout.FlexibleSpace();
	// 		GUILayout.EndHorizontal();
	// 		GUILayout.Space(8f);
	// 	}
	// 	
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
