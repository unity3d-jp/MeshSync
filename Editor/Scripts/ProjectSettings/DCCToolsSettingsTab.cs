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
            Button autoDetectButton = containerInstance.Query<Button>("AutoDetect").First();
            autoDetectButton.clickable.clicked += OnAutoDetectButtonClicked;
            Button addDCCToolButton = containerInstance.Query<Button>("AddDCCToolButton").First();
            addDCCToolButton.clickable.clicked += OnAddDCCToolButtonClicked;

            
            //Add the container of this tab to root
            root.Add(containerInstance);
        }

//----------------------------------------------------------------------------------------------------------------------        

        void AddDCCToolSettingsContainer(DCCToolInfo info, VisualElement top, VisualTreeAsset dccToolInfoTemplate) {
            TemplateContainer container = dccToolInfoTemplate.CloneTree();
            container.Query<Label>("DCCToolName").First().text = info.Type.ToString() + " " + info.Version;
            container.Query<Label>("DCCToolPath").First().text = "Path to " + info.Type.ToString();

            if (string.IsNullOrEmpty(info.PluginVersion)) {
                container.Query<Label>("DCCToolStatus").First().text = "Plugin not installed";
            }
            
            top.Add(container);
        }
        
//----------------------------------------------------------------------------------------------------------------------        

        #region Button callbacks
        void OnAddDCCToolButtonClicked() {
            //[TODO-sin: 2020-4-24] Show window to add  ?
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            if (settings.AddDCCToolInfo("Test", DCCToolType.AUTODESK_MAYA, "2020")) {
                Setup(m_root);
            }
            
            
        }
        
        void OnAutoDetectButtonClicked() {
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            if (settings.AddInstalledDCCTools()) {
                Setup(m_root);
            }
        }
        #endregion


        private VisualElement m_root = null;
    }
    
} //end namespace
