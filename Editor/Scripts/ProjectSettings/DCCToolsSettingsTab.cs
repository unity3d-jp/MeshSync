using UnityEditor;
using UnityEditorInternal;

using System.IO;
using System.Linq;
using System.Collections.Generic;
using NUnit.Framework;
using Unity.AnimeToolbox;
using UnityEngine;
using UnityEngine.UIElements;


namespace UnityEditor.MeshSync {
	internal class DCCToolsSettingsTab : IMeshSyncSettingsTab{

        
        public DCCToolsSettingsTab() {
            
            
        }

//----------------------------------------------------------------------------------------------------------------------        
        public void Setup(VisualElement root) {
            
            m_root = root;
            m_root.Clear();
            
            VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
                Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "DCCToolsSettings_Container")
            );
            
            VisualTreeAsset dccToolInfoTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
                Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "DCCToolInfoTemplate")
            );

            TemplateContainer containerInstance = container.CloneTree();
            ScrollView scrollView = containerInstance.Query<ScrollView>().First();

            //[TODO-sin: 2020-4-24] Auto detect installed DCC tools + check MeshSync status
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            foreach (var dccToolInfo in settings.GetDCCToolInfos()) {
                AddDCCToolSettingsContainer(dccToolInfo.Value, scrollView, dccToolInfoTemplate);                
            }
            
            //Buttons
            Button autoDetectButton = containerInstance.Query<Button>("AutoDetectButton").First();
            autoDetectButton.clickable.clicked += OnAutoDetectButtonClicked;
            Button addDCCToolButton = containerInstance.Query<Button>("AddDCCToolButton").First();
            addDCCToolButton.clickable.clicked += OnAddDCCToolButtonClicked;

            
            //Add the container of this tab to root
            root.Add(containerInstance);
        }

//----------------------------------------------------------------------------------------------------------------------        

        void AddDCCToolSettingsContainer(DCCToolInfo dccToolInfo, VisualElement top, VisualTreeAsset dccToolInfoTemplate) {
            TemplateContainer container = dccToolInfoTemplate.CloneTree();
            Label nameLabel = container.Query<Label>("DCCToolName").First();
            nameLabel.text = dccToolInfo.GetDescription();
            
            container.Query<Label>("DCCToolPath").First().text = "Path: " + dccToolInfo.AppPath;

            
            BaseDCCIntegrator integrator = DCCIntegratorFactory.Create(dccToolInfo);
            DCCPluginInstallInfo installInfo = integrator.FindInstallInfo();

            Label statusLabel = container.Query<Label>("DCCToolStatus").First();
            if (null==installInfo || string.IsNullOrEmpty(installInfo.PluginVersion)) {
                statusLabel.text = "MeshSync Plugin not installed";
            } else {
                statusLabel.AddToClassList("plugin-installed");
                statusLabel.text = "MeshSync Plugin installed. Version: " + installInfo.PluginVersion;
            }

            //Buttons
            {
                Button button = container.Query<Button>("RemoveDCCToolButton").First();
                button.clickable.clickedWithEventInfo += OnRemoveDCCToolButtonClicked;
                button.userData = dccToolInfo;
            }

            
            {
                Button button = container.Query<Button>("InstallPluginButton").First();
                button.clickable.clickedWithEventInfo += OnInstallPluginButtonClicked;
                button.userData = integrator;
            }
            
            top.Add(container);
        }
        
//----------------------------------------------------------------------------------------------------------------------        

        #region Button callbacks
        void OnAddDCCToolButtonClicked() {
            string folder = EditorUtility.OpenFolderPanel("Add DCC Tool", m_lastOpenedFolder, "");
            if (string.IsNullOrEmpty(folder)) {
                return;
            }

            m_lastOpenedFolder = folder;

            //Find the path to the actual app
            DCCToolType lastDCCToolType = DCCToolType.AUTODESK_MAYA;
            DCCToolInfo dccToolInfo = null;
            for (int i = 0; i < (int) (DCCToolType.NUM_DCC_TOOL_TYPES) && null==dccToolInfo; ++i) {
                lastDCCToolType = (DCCToolType) (i);
                dccToolInfo = DCCFinderUtility.FindDCCToolInDirectory(lastDCCToolType, null, m_lastOpenedFolder);
            }

            if (null==dccToolInfo) {
                EditorUtility.DisplayDialog("MeshSync Project Settings", "No DCC Tool is detected", "Ok");
                return;
            }
            
            //Add
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            if (settings.AddDCCTool(dccToolInfo)) {
                Setup(m_root);
            }
            
            
        }
        
        void OnAutoDetectButtonClicked() {
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            if (settings.AddInstalledDCCTools()) {
                Setup(m_root);
            }
        }

        
        void OnRemoveDCCToolButtonClicked(EventBase evt) {
            Button button = evt.target as Button;
            if (null == button) {
                Debug.LogWarning("[MeshSync] Failed to Remove DCC Tool");
                return;
            }

            DCCToolInfo info = button.userData as DCCToolInfo;
            if (null==info || string.IsNullOrEmpty(info.AppPath)) {
                Debug.LogWarning("[MeshSync] Failed to Remove DCC Tool");
                return;
            }
            
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            if (settings.RemoveDCCTool(info.AppPath)) {
                Setup(m_root);
            }
            
        }
        void OnInstallPluginButtonClicked(EventBase evt) {
            
            Button button = evt.target as Button;
            if (null == button) {
                Debug.LogWarning("[MeshSync] Failed to Install Plugin");
                return;
            }

            BaseDCCIntegrator integrator = button.userData as BaseDCCIntegrator;
            if (null==integrator) {
                Debug.LogWarning("[MeshSync] Failed to Install Plugin");
                return;
            }

            integrator.Integrate(() =>
            {
                DCCToolInfo dccToolInfo = integrator.GetDCCToolInfo();
                EditorUtility.DisplayDialog("MeshSync",
                    "MeshSync plugin installed for " + dccToolInfo.GetDescription(), 
                    "Ok"
                );                
                Setup(m_root);
            });

        }
        #endregion

//----------------------------------------------------------------------------------------------------------------------        

        private VisualElement m_root = null;
        private string m_lastOpenedFolder = "";

    }
    
} //end namespace
