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

        void AddDCCToolSettingsContainer(DCCToolInfo info, VisualElement top, VisualTreeAsset dccToolInfoTemplate) {
            TemplateContainer container = dccToolInfoTemplate.CloneTree();
            Label nameLabel = container.Query<Label>("DCCToolName").First();
            switch (info.Type) {
                case DCCToolType.AUTODESK_MAYA: {
                    nameLabel.text = "Maya " + info.Version;
                    break;
                }
                case DCCToolType.AUTODESK_3DSMAX: {
                    nameLabel.text = "3DS Max " + info.Version;
                    break;
                }
            }
            
            container.Query<Label>("DCCToolPath").First().text = "Path: " + info.AppPath;

            Label statusLabel = container.Query<Label>("DCCToolStatus").First();
            if (string.IsNullOrEmpty(info.PluginVersion)) {
                statusLabel.text = "MeshSync Plugin not installed";
            } else {
                statusLabel.AddToClassList("plugin-installed");
                statusLabel.text = "MeshSync Plugin installed";
            }

            //Buttons
            {
                Button button = container.Query<Button>("RemoveDCCToolButton").First();
                button.clickable.clickedWithEventInfo += OnRemoveDCCToolButtonClicked;
                button.userData = info.AppPath;
            }

            {
                Button button = container.Query<Button>("InstallPluginButton").First();
                button.clickable.clickedWithEventInfo += OnInstallPluginButtonClicked;
                button.userData = info.AppPath;
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
            string appPath = null;
            for (int i = 0; i < (int) (DCCToolType.NUM_DCC_TOOL_TYPES) && string.IsNullOrEmpty(appPath); ++i) {
                lastDCCToolType = (DCCToolType) (i);
                appPath = ProjectSettingsUtility.FindDCCToolAppPathInDirectory(lastDCCToolType, m_lastOpenedFolder);
            }

            if (string.IsNullOrEmpty(appPath)) {
                EditorUtility.DisplayDialog("MeshSync Project Settings", "No DCC Tool is detected", "Ok");
                return;
            }

            //Find version
            string version = null;
            switch (lastDCCToolType) {
                case DCCToolType.AUTODESK_MAYA: {
                    version = ProjectSettingsUtility.FindMayaVersion(appPath);
                    break;
                }
                case DCCToolType.AUTODESK_3DSMAX: {
                    version = ProjectSettingsUtility.Find3DSMaxVersion(appPath);
                    break;
                }
            }

            //Add
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            if (settings.AddDCCTool(appPath, lastDCCToolType, version)) {
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

            string appPath = button.userData as string;
            if (string.IsNullOrEmpty(appPath)) {
                Debug.LogWarning("[MeshSync] Failed to Remove DCC Tool");
                return;
            }
            
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            if (settings.RemoveDCCTool(appPath)) {
                Setup(m_root);
            }
            
        }
        void OnInstallPluginButtonClicked(EventBase evt) {
            
            //Check file if it exists. if it doesn't, try to download it
            
            //Copy the file to 
            // Windows:
            // If MAYA_APP_DIR environment variable is setup, copy the modules directory there.
            //     If not, go to %USERPROFILE%\Documents\maya in Windows Explorer, and copy the modules directory there.
            //     Mac:
            // Copy the UnityMeshSync directory and UnityMeshSync.mod file to /Users/Shared/Autodesk/modules/maya.
            //     Linux:
            // Copy the modules directory to ~/maya/<maya_version)            
            
            //Set configuration
            
            
            Debug.Log("Installing: " + evt.target);
        }
        #endregion

//----------------------------------------------------------------------------------------------------------------------        

        private VisualElement m_root = null;
        private string m_lastOpenedFolder = "";

    }
    
} //end namespace
