using System;
using System.Collections.Generic;
using System.IO;
using Unity.AnimeToolbox;
using Unity.AnimeToolbox.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;

namespace Unity.MeshSync.Editor {
	
class MeshSyncSettingsProvider : SettingsProvider {
	
	private class Contents {
		public static readonly GUIContent GeneralSettings = EditorGUIUtility.TrTextContent("Server");		
		public static readonly GUIContent SceneCachePlayer = EditorGUIUtility.TrTextContent("Scene Cache Player");		
	}
	
//----------------------------------------------------------------------------------------------------------------------	
	
	MeshSyncSettingsProvider() : base(PROJECT_SETTINGS_MENU_PATH,SettingsScope.Project) {
		m_tabs = new IMeshSyncSettingsTab[MeshSyncEditorConstants.MAX_SETTINGS_TAB];
		Button[] tabButtons = new Button[MeshSyncEditorConstants.MAX_SETTINGS_TAB];		
		m_tabs[MeshSyncEditorConstants.GENERAL_SETTINGS_TAB] = new GeneralSettingsTab();
		m_tabs[MeshSyncEditorConstants.SCENE_CACHE_PLAYER_SETTINGS_TAB] = new SceneCachePlayerSettingsTab();
		
		//activateHandler is called when the user clicks on the Settings item in the Settings window.
		activateHandler = (string searchContext, VisualElement root) => {

			
			//Main Tree
			VisualTreeAsset main = UIElementsEditorUtility.LoadVisualTreeAsset(
				Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "ProjectSettings_Main")
			);
			main.CloneTree(root);

			//Tab Buttons
			VisualElement tabsContainer = root.Query<VisualElement>("TabsContainer");
			VisualTreeAsset tabButtonTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
				Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "TabButtonTemplate")
			);

			tabButtons[0] = CreateButton(tabButtonTemplate, Contents.GeneralSettings, OnGeneralSettingsTabClicked);
			tabButtons[1] = CreateButton(tabButtonTemplate, Contents.SceneCachePlayer, OnSceneCachePlayerTabClicked);			

			foreach (Button tabButton in tabButtons) {
				tabsContainer.Add(tabButton);
			}
			
			//Style
			UIElementsEditorUtility.LoadAndAddStyle(
				root.styleSheets,
				Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH,"ProjectSettings_Style")
			);

			
			m_content = root.Query<VisualElement>("Content");
			UpdateSelectedTabButton(tabButtons[0]);
			SetupTab(MeshSyncEditorConstants.GENERAL_SETTINGS_TAB);
			
		};
		
		
		deactivateHandler = () => {
			SetupTab(MeshSyncEditorConstants.UNINITIALIZED_TAB);
		};

		//keywords
		HashSet<string> meshSyncKeywords = new HashSet<string>(new[] { "MeshSync",});
		meshSyncKeywords.UnionWith(GetSearchKeywordsFromGUIContentProperties<MeshSyncSettingsProvider.Contents>());
		meshSyncKeywords.UnionWith(GetSearchKeywordsFromGUIContentProperties<GeneralSettingsTab.Contents>());
		meshSyncKeywords.UnionWith(GetSearchKeywordsFromGUIContentProperties<SceneCachePlayerSettingsTab.Contents>());
		meshSyncKeywords.UnionWith(GetSearchKeywordsFromGUIContentProperties<MeshSyncPlayerConfigSection.Contents>());

		keywords = meshSyncKeywords;
		
	}


//----------------------------------------------------------------------------------------------------------------------	
	private Button CreateButton(VisualTreeAsset template, GUIContent content, Action<EventBase> onClicked) 
	{
		TemplateContainer container = template.CloneTree();
		Button button = container.Query<Button>().First();

		button.text = content.text;
		button.tooltip = content.tooltip;
		button.clickable.clickedWithEventInfo += onClicked;
		
		return button;
	}
	
//----------------------------------------------------------------------------------------------------------------------	

	#region Button Events
	
	static void OnGeneralSettingsTabClicked(EventBase evt) {
		if (!UpdateSelectedTabButton(evt.target as Button))
			return;

		m_settingsProvider.SetupTab(MeshSyncEditorConstants.GENERAL_SETTINGS_TAB);
		
	}
	
	static void OnSceneCachePlayerTabClicked(EventBase evt) {
		if (!UpdateSelectedTabButton(evt.target as Button))
			return;

		m_settingsProvider.SetupTab(MeshSyncEditorConstants.SCENE_CACHE_PLAYER_SETTINGS_TAB);
	}	

	#endregion	

//----------------------------------------------------------------------------------------------------------------------
	
	private void SetupTab(int tab) {
		if (tab == m_selectedTab)
			return;
		
		m_selectedTab = tab;
		m_content?.Clear();

		if (MeshSyncEditorConstants.UNINITIALIZED_TAB == m_selectedTab) {
			m_selectedTabButton = null;
			return;
		}
			
		m_tabs[m_selectedTab].Setup(m_content);
		
	}

//----------------------------------------------------------------------------------------------------------------------
	static bool UpdateSelectedTabButton(Button button) {
		if (null == button)
			return false;
		
		//Deactivate old button and activate selected button
		const string ACTIVE_TAB_BUTTON_CLASS = "tab-button-active";		
		m_selectedTabButton?.ToggleInClassList(ACTIVE_TAB_BUTTON_CLASS); //Deactivate the previous one
		m_selectedTabButton = button;
		m_selectedTabButton.ToggleInClassList(ACTIVE_TAB_BUTTON_CLASS);
		return true;
	}	
//----------------------------------------------------------------------------------------------------------------------

    [SettingsProvider]
    internal static SettingsProvider CreateMeshSyncSettingsProvider() {
	    m_settingsProvider = new MeshSyncSettingsProvider();
	    return m_settingsProvider;
    }
    
	
//----------------------------------------------------------------------------------------------------------------------

	private int m_selectedTab = MeshSyncEditorConstants.UNINITIALIZED_TAB;
	private readonly IMeshSyncSettingsTab[] m_tabs = null;

	private VisualElement m_content = null;

	private static MeshSyncSettingsProvider m_settingsProvider = null;

	private const string PROJECT_SETTINGS_MENU_PATH = "Project/MeshSync";

	
//----------------------------------------------------------------------------------------------------------------------
	
	private static Button m_selectedTabButton = null;
}

	
}
