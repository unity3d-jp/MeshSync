using Unity.AnimeToolbox.Editor;
using UnityEngine;
using UnityEngine.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;


namespace Unity.MeshSync.Editor {
internal class SceneCachePlayerSettingsTab : IMeshSyncSettingsTab {

	internal class Contents {
	}

//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
	    VisualTreeAsset tab = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH);
        TemplateContainer tabInstance = tab.CloneTree();
	    
	    VisualElement content = tabInstance.Query<VisualElement>("Content").First();
	    
	    MeshSyncPlayerConfigSection section = new MeshSyncPlayerConfigSection(MeshSyncPlayerType.CACHE_PLAYER);
	    
	    section.Setup(content);
	    
	    
	    root.Add(tabInstance);	    
    }

//----------------------------------------------------------------------------------------------------------------------

	
}

} //end namespace 
