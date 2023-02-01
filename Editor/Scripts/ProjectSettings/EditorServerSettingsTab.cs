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
        public static readonly GUIContent DefaultPort   = EditorGUIUtility.TrTextContent("Default Server Port");
        public static readonly GUIContent DefaultActive = EditorGUIUtility.TrTextContent("Start at Editor Launch");
        public static readonly GUIContent Port          = EditorGUIUtility.TrTextContent("Server Port");
    }

//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        Assert.IsNotNull(root);
        root.Clear();

        VisualTreeAsset   tab         = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.EDITOR_SERVER_SETTINGS_TAB_PATH);
        TemplateContainer tabInstance = tab.CloneTree();

        VisualElement content = tabInstance.Query<VisualElement>("Content").First();

        content.Add(m_statusLabel);

        m_portField =
            ProjectSettingsUtility.AddField<IntegerField, int>(content, Contents.Port, EditorServer.Port, null);

        m_portField.RegisterCallback<FocusOutEvent>(evt => {
            EditorServer.Port = (ushort)m_portField.value;
            EditorServer.ApplySettings();
        });

        //content.Add(new Label($"Default Settings"));
        //Add server port
        m_defaultPortField = ProjectSettingsUtility.AddField<IntegerField, int>(content, Contents.DefaultPort,
            EditorServerSettings.instance.Port, null
        );

        // Use Focus out event as the onValueChanged event is invoked while the user is typing
        m_defaultPortField.RegisterCallback<FocusOutEvent>(evt => { EditorServerSettings.instance.Port = (ushort)m_defaultPortField.value; });

        m_defaultActiveField = ProjectSettingsUtility.AddField<Toggle, bool>(content, Contents.DefaultActive, EditorServerSettings.instance.Active,
            (bool newValue) => { EditorServerSettings.instance.Active = newValue; });

        m_serveStatusButton = tabInstance.Query<Button>("Button").First();
        m_serveStatusButton.clicked += () => {
            EditorServer.Active = !EditorServer.Active;
            EditorServer.ApplySettings();
            UpdateStatus();
        };

        root.Add(tabInstance);

        UpdateStatus();
    }

    private void UpdateStatus() {
        bool   active = EditorServer.Active;
        string status = active ? "Started" : "Stopped";
        m_statusLabel.text = $"Editor Server (Status: {status})";

        m_serveStatusButton.text = active ? "Stop" : "Start";
    }

//----------------------------------------------------------------------------------------------------------------------

    private Label        m_status;
    private IntegerField m_portField;
    private IntegerField m_defaultPortField;
    private Toggle       m_defaultActiveField;
    private Button       m_serveStatusButton;
    private Label        m_statusLabel = new Label("Started");
}
} //end namespace 