using System;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine;
using UnityEngine.UIElements;
using UnityEditor;
using UnityEditor.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;

namespace Unity.MeshSync.Editor {
internal class EditorServerSettingsTab : IMeshSyncSettingsTab {
    private class Contents {

        public static readonly GUIContent ServerPort   = EditorGUIUtility.TrTextContent("Server port");
        public static readonly GUIContent Active = EditorGUIUtility.TrTextContent("Active");
    }
    
//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        Assert.IsNotNull(root);
        root.Clear();
        
        VisualTreeAsset tab = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.EDITOR_SERVER_SETTINGS_TAB_PATH);	    
        TemplateContainer tabInstance = tab.CloneTree();
      
        VisualElement content = tabInstance.Query<VisualElement>("Content").First();
        
        //Add server port
        m_serverPortField = AddField<IntegerField,int>(content, Contents.ServerPort,
            AutoSetupSettings.instance.Port, null
        );

        // Use Focus out event as the onValueChanged event is invoked while the user is typing
        m_serverPortField.RegisterCallback<FocusOutEvent>(evt => {
            AutoSetupSettings.instance.Port = (ushort)m_serverPortField.value;
            AutoSetup.ApplySettingsIfDirty();
        });
        
        m_activeField = AddField<Toggle, bool>(content, Contents.Active, AutoSetupSettings.instance.Active,
            (bool newValue) => {
                AutoSetupSettings.instance.Active = newValue;
                AutoSetup.ApplySettingsIfDirty();
            });

        root.Add(tabInstance);
    }

//----------------------------------------------------------------------------------------------------------------------
    
    //Support Toggle, FloatField, etc
    private F AddField<F,V>(VisualElement parent, GUIContent content,
        V initialValue, Action<V> onValueChanged) where F: VisualElement,INotifyValueChanged<V>, new()  
    {
        F field = UIElementsEditorUtility.AddField<F, V>(parent, content, initialValue, (ChangeEvent<V> changeEvent) => {
            onValueChanged?.Invoke(changeEvent.newValue);
        });

        field.AddToClassList("project-settings-field");
        return field;
    }

//----------------------------------------------------------------------------------------------------------------------

    private IntegerField m_serverPortField;
    private Toggle       m_activeField;
}

} //end namespace 
