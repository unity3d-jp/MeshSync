using System;
using System.Collections.Generic;
using System.IO;
using Unity.AnimeToolbox;
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
		m_tabButtons = new Button[MeshSyncEditorConstants.MAX_SETTINGS_TAB];		
		m_tabs[MeshSyncEditorConstants.GENERAL_SETTINGS_TAB] = new GeneralSettingsTab();
		m_tabs[MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_TAB] = new DCCToolsSettingsTab();
		
		//activateHandler is called when the user clicks on the Settings item in the Settings window.
		activateHandler = (string searchContext, VisualElement rootElement) => {

			var rootElement1 = rootElement;
			
			//Main Tree
			VisualTreeAsset main = UIElementsEditorUtility.LoadVisualTreeAsset(
				Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "ProjectSettings_Main")
			);
			main.CloneTree(rootElement1);


			//Tab Buttons
			VisualElement tabsContainer = rootElement1.Query<VisualElement>("TabsContainer");
			VisualTreeAsset tabButtonTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
				Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "TabButtonTemplate")
			);

			m_tabButtons[0] = CreateButton(tabButtonTemplate, "General Settings", OnGeneralSettingsTabClicked);
			m_tabButtons[1] = CreateButton(tabButtonTemplate, "DCC Tools", OnDCCToolsTabClicked);

			foreach (Button tabButton in m_tabButtons) {
				tabsContainer.Add(tabButton);
			}
			
			//Style
			UIElementsEditorUtility.LoadAndAddStyle(
				rootElement1.styleSheets,
				Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH,"ProjectSettings_Style")
			);

			m_content = rootElement1.Query<VisualElement>("Content");
			SetupTab(MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_TAB);
			
		};
		
		deactivateHandler = () => {
			SetupTab(MeshSyncEditorConstants.UNINITIALIZED_TAB);
		};

		//TODO-sin: 2020-4-24 Fill in more keywords
		keywords = new HashSet<string>(new[] {
			"MeshSync",
		});
		
	}


//----------------------------------------------------------------------------------------------------------------------	
	private Button CreateButton(VisualTreeAsset template, string labelText, Action onClicked) 
	{
		TemplateContainer container = template.CloneTree();
		Button button = container.Query<Button>().First();

		button.text = labelText;
		button.clickable.clicked += onClicked;
		
		return button;
	}
	
//----------------------------------------------------------------------------------------------------------------------	

	#region Button Events
	
	static void OnGeneralSettingsTabClicked() {
		m_settingsProvider.SetupTab(MeshSyncEditorConstants.GENERAL_SETTINGS_TAB);
	}

	static void OnDCCToolsTabClicked() {
		m_settingsProvider.SetupTab(MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_TAB);
	}
	#endregion	

//----------------------------------------------------------------------------------------------------------------------
	
	private void SetupTab(int tab) {
		if (tab == m_selectedTab)
			return;

		const string ACTIVE_TAB_BUTTON_CLASS = "tab-button-active";
		
		m_selectedTabButton?.ToggleInClassList(ACTIVE_TAB_BUTTON_CLASS); //Deactivate the previous one
		
		m_selectedTab = tab;
		m_content.Clear();

		if (MeshSyncEditorConstants.UNINITIALIZED_TAB == m_selectedTab) {
			m_selectedTabButton = null;
			return;
		}
		
		//Activate the selected tab button
		m_selectedTabButton = m_tabButtons[m_selectedTab];
		m_selectedTabButton.ToggleInClassList(ACTIVE_TAB_BUTTON_CLASS); 
		
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
	private readonly IMeshSyncSettingsTab[] m_tabs = null;
	private Button m_selectedTabButton = null;
	private readonly Button[] m_tabButtons = null;

	private VisualElement m_content = null;

	private static MeshSyncSettingsProvider m_settingsProvider = null;

	private const string PROJECT_SETTINGS_MENU_PATH = "Project/MeshSync";
}

	
}
