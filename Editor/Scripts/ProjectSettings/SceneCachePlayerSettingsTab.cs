using NUnit.Framework;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;


namespace Unity.MeshSync.Editor {
internal class SceneCachePlayerSettingsTab : IMeshSyncSettingsTab {

    internal class Contents {
        public static readonly GUIContent OutputPrefabPath = EditorGUIUtility.TrTextContent("Scene Cache Output Path");
    }

//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        Assert.IsNotNull(root);
        
        VisualTreeAsset tab = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SCENE_CACHE_PLAYER_SETTINGS_TAB_PATH);
        TemplateContainer tabInstance = tab.CloneTree();
        
        VisualElement content = tabInstance.Query<VisualElement>("Content").First();
        
        m_generatedSCResPathTextField	 = tabInstance.Query<TextField>("GeneratedSCResPathText").First();
        m_generatedSCResPathTextField.RegisterValueChangedCallback((ChangeEvent<string> changeEvent) => {
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            settings.SetSceneCacheOutputPath(changeEvent.newValue);
            settings.Save();
        });        
        
        m_outputPathSelectButton = tabInstance.Query<Button>("OutputPathSelectButton").First();
        m_outputPathSelectButton.clicked += OnOutputPathSelectButtonClicked;
        RefreshSettings();    
        
        //MeshSyncPlayerConfig
        MeshSyncPlayerConfigSection section = new MeshSyncPlayerConfigSection(MeshSyncPlayerType.CACHE_PLAYER);	    
        section.Setup(content);
                
        root.Add(tabInstance);	    
    }
    
//----------------------------------------------------------------------------------------------------------------------
    void OnOutputPathSelectButtonClicked() {
        string path = EditorUtility.OpenFolderPanel("Select Scene Cache Output Path",
                                                    m_generatedSCResPathTextField.value,
                                                    "");

        if (string.IsNullOrEmpty(path))
            return;

        if (!path.StartsWith(Application.dataPath)) {
            EditorUtility.DisplayDialog("MeshSync",
                $"Invalid path: {path}. " + 
                "Path has to be under the Assets folder.", 
                "Ok");
            return;            
        }        

        MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
        
        path = AssetEditorUtility.NormalizePath(path);
        settings.SetSceneCacheOutputPath(path);
        settings.Save();

        RefreshSettings();
    }
//----------------------------------------------------------------------------------------------------------------------

    void RefreshSettings() {
        MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
        m_generatedSCResPathTextField.value = settings.GetSceneCacheOutputPath();		
    }

    
//----------------------------------------------------------------------------------------------------------------------
    
    private TextField m_generatedSCResPathTextField  	= null;
    private Button    m_outputPathSelectButton 	= null;

}

} //end namespace 
