using Unity.AnimeToolbox.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;


namespace Unity.MeshSync.Editor {
internal class SceneCachePlayerSettingsTab : IMeshSyncSettingsTab {

    internal class Contents {
        public static readonly GUIContent OutputPrefabPath = EditorGUIUtility.TrTextContent("Output prefab path");
    }

//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        VisualTreeAsset tab = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH);
        TemplateContainer tabInstance = tab.CloneTree();
        
        VisualElement content = tabInstance.Query<VisualElement>("Content").First();
        
        m_prefabPathTextField	 = tabInstance.Query<TextField>("PrefabPathText").First();
        m_prefabPathSelectButton = tabInstance.Query<Button>("PrefabPathSelectButton").First();
        m_prefabPathSelectButton.clicked += OnPrefabPathSelectButtonClicked;
        RefreshSettings();    
        
        //MeshSyncPlayerConfig
        MeshSyncPlayerConfigSection section = new MeshSyncPlayerConfigSection(MeshSyncPlayerType.CACHE_PLAYER);	    
        section.Setup(content);
                
        root.Add(tabInstance);	    
    }
    
//----------------------------------------------------------------------------------------------------------------------
    void OnPrefabPathSelectButtonClicked() {
        string path = EditorUtility.OpenFolderPanel("Select Output SceneCache Prefab Path"
                                                    ,m_prefabPathTextField.value,"");

        if (string.IsNullOrEmpty(path))
            return;

        MeshSyncRuntimeSettings settings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        
        path = AssetUtility.NormalizeAssetPath(path);		
        settings.SetOutputSceneCachePrefabPath(path);

        RefreshSettings();
    }
//----------------------------------------------------------------------------------------------------------------------

    void RefreshSettings() {
        MeshSyncRuntimeSettings settings = MeshSyncRuntimeSettings.GetOrCreateSettings();
        m_prefabPathTextField.value = settings.GetOutputSceneCachePrefabPath();		
    }

    
//----------------------------------------------------------------------------------------------------------------------
    
    private TextField m_prefabPathTextField  	= null;
    private Button    m_prefabPathSelectButton 	= null;

}

} //end namespace 
