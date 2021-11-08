using System;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine;
using UnityEngine.UIElements;
using UnityEditor;
using UnityEditor.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;

namespace Unity.MeshSync.Editor {
internal class ServerSettingsTab : IMeshSyncSettingsTab {

    internal class Contents {

        public static readonly GUIContent ServerPort = EditorGUIUtility.TrTextContent("Server port");
        public static readonly GUIContent AllowPublicAccess = EditorGUIUtility.TrTextContent("Allow public access");
        
    }


//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        Assert.IsNotNull(root);
        root.Clear();
        
        VisualTreeAsset tab = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SERVER_SETTINGS_TAB_PATH);	    
        TemplateContainer tabInstance = tab.CloneTree();
      
        VisualElement content = tabInstance.Query<VisualElement>("Content").First();
        
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();
        
        //Add server port
        m_serverPortField = AddField<IntegerField,int>(content, Contents.ServerPort,
            projectSettings.GetDefaultServerPort(),
            (int newValue) => { projectSettings.SetDefaultServerPort((ushort) newValue); }
        );

        m_allowPublicAccessToggle = AddField<Toggle,bool>(content, Contents.AllowPublicAccess,
            projectSettings.GetServerPublicAccess(),
            (bool  newValue) => { projectSettings.SetServerPublicAccess(newValue); }
        );
        
        //MeshSyncPlayerConfig section
        MeshSyncPlayerConfigSection section = new MeshSyncPlayerConfigSection(MeshSyncPlayerType.SERVER);
        section.Setup(content);
        
        Button resetButton = tabInstance.Query<Button>("ResetButton").First();
        resetButton.clicked += () => {
            projectSettings.ResetDefaultServerConfig();
            projectSettings.Save();
            Setup(root);
        };
        
        
        root.Add(tabInstance);
    }

//----------------------------------------------------------------------------------------------------------------------
    
    //Support Toggle, FloatField, etc
    private F AddField<F,V>(VisualElement parent, GUIContent content,
        V initialValue, Action<V> onValueChanged) where F: VisualElement,INotifyValueChanged<V>, new()  
    {
        F field = UIElementsEditorUtility.AddField<F, V>(parent, content, initialValue, (ChangeEvent<V> changeEvent) => {
            onValueChanged(changeEvent.newValue);
            MeshSyncProjectSettings.GetOrCreateSettings().Save();
        });

        field.AddToClassList("project-settings-field");
        return field;
    }	
    
//----------------------------------------------------------------------------------------------------------------------
    private VisualTreeAsset LoadVisualTreeAsset(string path) {
        return UIElementsEditorUtility.LoadVisualTreeAsset(path); 
    } 

//----------------------------------------------------------------------------------------------------------------------
    
    private IntegerField m_serverPortField;
    private Toggle m_allowPublicAccessToggle;
    
    
}

} //end namespace 
