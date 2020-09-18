using System;
using Unity.AnimeToolbox.Editor;
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
	    
		VisualTreeAsset tab = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SERVER_SETTINGS_TAB_PATH);	    
		TemplateContainer tabInstance = tab.CloneTree();
      
	    VisualElement content = tabInstance.Query<VisualElement>("Content").First();
	    

	    //Templates
	    VisualTreeAsset fieldTemplate = LoadVisualTreeAsset(Constants.PROJECT_SETTINGS_FIELD_TEMPLATE_PATH);
	    MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
	    
	    //Add server port
	    m_serverPortField = AddField<IntegerField,int>(fieldTemplate, content, Contents.ServerPort,
		    runtimeSettings.GetDefaultServerPort(),
		    (int newValue) => {
			    MeshSyncRuntimeSettings settings = MeshSyncRuntimeSettings.GetOrCreateSettings();
			    settings.SetDefaultServerPort((ushort) newValue);
		    }
	    );

	    m_allowPublicAccessToggle = AddField<Toggle,bool>(fieldTemplate, content, Contents.AllowPublicAccess,
		    runtimeSettings.GetServerPublicAccess(),
		    (bool  newValue) => {
			    MeshSyncRuntimeSettings settings = MeshSyncRuntimeSettings.GetOrCreateSettings();
			    settings.SetServerPublicAccess(newValue);
		    }
	    );
	    

	    //MeshSyncPlayerConfig section
	    MeshSyncPlayerConfigSection section = new MeshSyncPlayerConfigSection(MeshSyncPlayerType.SERVER);
	    section.Setup(content);
	    
      
	            
	    
	    
        root.Add(tabInstance);
    }

//----------------------------------------------------------------------------------------------------------------------
	
	//Support Toggle, FloatField, etc
	private F AddField<F,V>(VisualTreeAsset template, VisualElement parent, GUIContent content,
		V initialValue, Action<V> onValueChanged) where F: VisualElement,INotifyValueChanged<V>, new()  
	{
		TemplateContainer templateInstance = template.CloneTree();
		VisualElement     fieldContainer   = templateInstance.Query<VisualElement>("FieldContainer").First();
		Label             label            = templateInstance.Query<Label>().First();
		label.text    = content.text;
		label.tooltip = content.tooltip;
        
		F field = new F();
		field.AddToClassList("project-settings-field");
		field.SetValueWithoutNotify(initialValue);
		field.RegisterValueChangedCallback((ChangeEvent<V> changeEvent) => {
        
			onValueChanged(changeEvent.newValue);
			MeshSyncRuntimeSettings.GetOrCreateSettings().SaveSettings();
		});        
        
		fieldContainer.Add(field);
		parent.Add(templateInstance);
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
