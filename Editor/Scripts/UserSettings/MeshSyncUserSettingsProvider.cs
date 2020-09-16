using System.Collections.Generic;
using Unity.AnimeToolbox.Editor;
using UnityEditor;
using UnityEngine.Assertions;
using UnityEngine.UIElements;

namespace Unity.MeshSync.Editor {
internal class MeshSyncUserSettingsProvider : SettingsProvider {
	
		
//----------------------------------------------------------------------------------------------------------------------	

	private MeshSyncUserSettingsProvider() : base(USER_SETTINGS_MENU_PATH,SettingsScope.User) {
		
	
		//activateHandler is called when the user clicks on the Settings item in the Settings window.
		
		activateHandler = (string searchContext, VisualElement root) => {

			
			//Main Tree
			VisualTreeAsset main = UIElementsEditorUtility.LoadVisualTreeAsset(MeshSyncEditorConstants.MAIN_USER_SETTINGS_PATH);
			Assert.IsNotNull(main);
			main.CloneTree(root);			
			
			//Style
			UIElementsEditorUtility.LoadAndAddStyle( root.styleSheets, MeshSyncEditorConstants.USER_SETTINGS_STYLE_PATH);	

		};
				
		deactivateHandler = () => {
		};

		//keywords
		HashSet<string> meshSyncKeywords = new HashSet<string>(new[] { "MeshSync",});
//		sisKeywords.UnionWith(GetSearchKeywordsFromGUIContentProperties<MeshSyncUserSettingsProvider.Contents>());

		keywords = meshSyncKeywords;
		
	}


	

//----------------------------------------------------------------------------------------------------------------------

    [SettingsProvider]
    internal static SettingsProvider CreateMeshSyncUserSettingsProvider() {
	    m_settingsProvider = new MeshSyncUserSettingsProvider();
	    return m_settingsProvider;
    }
    
	
//----------------------------------------------------------------------------------------------------------------------

	private static MeshSyncUserSettingsProvider m_settingsProvider = null;
	private const  string USER_SETTINGS_MENU_PATH = "Preferences/MeshSync";
	
	

//----------------------------------------------------------------------------------------------------------------------

}

	
}
