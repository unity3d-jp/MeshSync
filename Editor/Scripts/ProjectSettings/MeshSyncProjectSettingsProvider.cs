using System;
using System.Collections.Generic;
using System.IO;
using Unity.AnimeToolbox;
using Unity.AnimeToolbox.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;

namespace Unity.MeshSync.Editor {
	
class MeshSyncProjectSettingsProvider : SettingsProvider {
	
	private class Contents {
		public static readonly GUIContent Server = EditorGUIUtility.TrTextContent("Server");		
		public static readonly GUIContent SceneCachePlayer = EditorGUIUtility.TrTextContent("Scene Cache Player");		
	}
	
//----------------------------------------------------------------------------------------------------------------------	
	
	MeshSyncProjectSettingsProvider() : base(PROJECT_SETTINGS_MENU_PATH,SettingsScope.Project) {
		m_tabs = new IMeshSyncSettingsTab[MeshSyncEditorConstants.MAX_SETTINGS_TAB];
		Button[] tabButtons = new Button[MeshSyncEditorConstants.MAX_SETTINGS_TAB];		
		m_tabs[MeshSyncEditorConstants.SERVER_SETTINGS_TAB] = new ServerSettingsTab();
		m_tabs[MeshSyncEditorConstants.SCENE_CACHE_PLAYER_SETTINGS_TAB] = new SceneCachePlayerSettingsTab();
		
		//activateHandler is called when the user clicks on the Settings item in the Settings window.
		activateHandler = (string searchContext, VisualElement root) => {
			
			//Main Tree
			VisualTreeAsset main = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.MAIN_PROJECT_SETTINGS_PATH);
			main.CloneTree(root);

			//Tab Buttons
			VisualElement tabsContainer = root.Query<VisualElement>("TabsContainer");
			VisualTreeAsset btnTemplate=UIElementsEditorUtility.LoadVisualTreeAsset(Constants.TAB_BUTTON_TEMPLATE_PATH);

			tabButtons[0] = CreateButton(btnTemplate, Contents.Server, OnServerSettingsTabClicked);
			tabButtons[1] = CreateButton(btnTemplate, Contents.SceneCachePlayer, OnSceneCachePlayerTabClicked);			

			foreach (Button tabButton in tabButtons) {
				tabsContainer.Add(tabButton);
			}
			
			//Style
			UIElementsEditorUtility.LoadAndAddStyle(root.styleSheets, Constants.PROJECT_SETTINGS_STYLE_PATH);

			
			m_content = root.Query<VisualElement>("Content");
			UpdateSelectedTabButton(tabButtons[0]);
			SetupTab(MeshSyncEditorConstants.SERVER_SETTINGS_TAB);
			
		};
		
		
		deactivateHandler = () => {
			SetupTab(MeshSyncEditorConstants.UNINITIALIZED_TAB);
		};

		//keywords
		HashSet<string> meshSyncKeywords = new HashSet<string>(new[] { "MeshSync",});
		meshSyncKeywords.UnionWith(GetSearchKeywordsFromGUIContentProperties<MeshSyncProjectSettingsProvider.Contents>());
		meshSyncKeywords.UnionWith(GetSearchKeywordsFromGUIContentProperties<ServerSettingsTab.Contents>());
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
	
	static void OnServerSettingsTabClicked(EventBase evt) {
		if (!UpdateSelectedTabButton(evt.target as Button))
			return;

		m_projectSettingsProvider.SetupTab(MeshSyncEditorConstants.SERVER_SETTINGS_TAB);
		
	}
	
	static void OnSceneCachePlayerTabClicked(EventBase evt) {
		if (!UpdateSelectedTabButton(evt.target as Button))
			return;

		m_projectSettingsProvider.SetupTab(MeshSyncEditorConstants.SCENE_CACHE_PLAYER_SETTINGS_TAB);
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
	    m_projectSettingsProvider = new MeshSyncProjectSettingsProvider();
	    return m_projectSettingsProvider;
    }
    
	
//----------------------------------------------------------------------------------------------------------------------

	private int m_selectedTab = MeshSyncEditorConstants.UNINITIALIZED_TAB;
	private readonly IMeshSyncSettingsTab[] m_tabs = null;

	private VisualElement m_content = null;

	private static MeshSyncProjectSettingsProvider m_projectSettingsProvider = null;

	private const string PROJECT_SETTINGS_MENU_PATH = "Project/MeshSync";

	
//----------------------------------------------------------------------------------------------------------------------
	
	private static Button m_selectedTabButton = null;
}

	
}
