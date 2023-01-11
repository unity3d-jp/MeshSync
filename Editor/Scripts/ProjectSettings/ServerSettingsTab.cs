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
        public static readonly GUIContent ServerPort        = EditorGUIUtility.TrTextContent("Server port");
        public static readonly GUIContent AllowPublicAccess = EditorGUIUtility.TrTextContent("Allow public access");
    }


//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        Assert.IsNotNull(root);
        root.Clear();

        VisualTreeAsset   tab         = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SERVER_SETTINGS_TAB_PATH);
        TemplateContainer tabInstance = tab.CloneTree();

        VisualElement content = tabInstance.Query<VisualElement>("Content").First();

        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateInstance();

        //Add server port
        m_serverPortField = ProjectSettingsUtility.AddField<IntegerField, int>(content, Contents.ServerPort,
            projectSettings.GetDefaultServerPort(),
            (int newValue) => {
                projectSettings.SetDefaultServerPort((ushort)newValue);
                MeshSyncProjectSettings.GetOrCreateInstance().SaveInEditor();
            }
        );

        m_allowPublicAccessToggle = ProjectSettingsUtility.AddField<Toggle, bool>(content, Contents.AllowPublicAccess,
            projectSettings.GetServerPublicAccess(),
            (bool  newValue) => {
                projectSettings.SetServerPublicAccess(newValue);
                MeshSyncProjectSettings.GetOrCreateInstance().SaveInEditor();
            }
        );

        //MeshSyncPlayerConfig section
        MeshSyncPlayerConfigSection section = new MeshSyncPlayerConfigSection(MeshSyncPlayerType.SERVER);
        section.Setup(content);

        Button resetButton = tabInstance.Query<Button>("ResetButton").First();
        resetButton.clicked += () => {
            projectSettings.ResetDefaultServerConfig();
            projectSettings.SaveInEditor();
            Setup(root);
        };


        root.Add(tabInstance);
    }

//----------------------------------------------------------------------------------------------------------------------
    private VisualTreeAsset LoadVisualTreeAsset(string path) {
        return UIElementsEditorUtility.LoadVisualTreeAsset(path);
    }

//----------------------------------------------------------------------------------------------------------------------

    private IntegerField m_serverPortField;
    private Toggle       m_allowPublicAccessToggle;
}
} //end namespace 