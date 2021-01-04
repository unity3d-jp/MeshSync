using System.Collections.Generic;
using System.IO;
using Unity.AnimeToolbox;
using Unity.AnimeToolbox.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;


namespace Unity.MeshSync.Editor {
	internal class DCCToolsSettingsTab : IMeshSyncSettingsTab{

        
        public DCCToolsSettingsTab() {
            
            
        }

//----------------------------------------------------------------------------------------------------------------------        
        public void Setup(VisualElement root) {

            m_dccStatusLabels.Clear();
            m_dccContainers.Clear();
            
            m_root = root;
            m_root.Clear();

            VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
                MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_CONTAINER_PATH
            );
            
            VisualTreeAsset dccToolInfoTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
                MeshSyncEditorConstants.DCC_TOOL_INFO_TEMPLATE_PATH
            );

            TemplateContainer containerInstance = container.CloneTree();
            ScrollView scrollView = containerInstance.Query<ScrollView>().First();

            //[TODO-sin: 2020-4-24] Auto detect installed DCC tools + check MeshSync status
            MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
            foreach (var dccToolInfo in settings.GetDCCToolInfos()) {
                AddDCCToolSettingsContainer(dccToolInfo.Value, scrollView, dccToolInfoTemplate);                
            }
            
            //Buttons
            Button autoDetectDCCButton = containerInstance.Query<Button>("AutoDetectDCCButton").First();
            autoDetectDCCButton.clickable.clicked += OnAutoDetectButtonClicked;
            Button addDCCToolButton = containerInstance.Query<Button>("AddDCCToolButton").First();
            addDCCToolButton.clickable.clicked += OnAddDCCToolButtonClicked;

            
            //Add the container of this tab to root
            root.Add(containerInstance);
        }

//----------------------------------------------------------------------------------------------------------------------        

        private void AddDCCToolSettingsContainer(DCCToolInfo dccToolInfo, VisualElement top, VisualTreeAsset dccToolInfoTemplate) {
            string desc = dccToolInfo.GetDescription();
            TemplateContainer container = dccToolInfoTemplate.CloneTree();
            Label nameLabel = container.Query<Label>("DCCToolName").First();
            nameLabel.text = desc;
            
            //Load icon
            Texture2D iconTex = LoadIcon(dccToolInfo.IconPath);
            if (null != iconTex) {
                container.Query<Image>("DCCToolImage").First().image = iconTex;
            } else {
                container.Query<Label>("DCCToolImageLabel").First().text = desc[0].ToString();
            }
            
            container.Query<Label>("DCCToolPath").First().text = "Path: " + dccToolInfo.AppPath;

            BaseDCCIntegrator integrator = DCCIntegratorFactory.Create(dccToolInfo);


            Label  statusLabel = container.Query<Label>("DCCToolStatus").First();
            UpdateDCCPluginStatus(integrator, statusLabel);
            
            m_dccStatusLabels[dccToolInfo.AppPath] = statusLabel;
            m_dccContainers[dccToolInfo.AppPath]   = container; 
                
                
            //Buttons
            {
                Button button = container.Query<Button>("LaunchDCCToolButton").First();
                button.clickable.clickedWithEventInfo += OnLaunchDCCToolButtonClicked;
                button.userData                       =  dccToolInfo;
            }
            {
                Button button = container.Query<Button>("InstallPluginButton").First();
                button.clickable.clickedWithEventInfo += OnInstallPluginButtonClicked;
                button.userData                       =  integrator;
            }
            {
                Button button = container.Query<Button>("RemoveDCCToolButton").First();
                button.clickable.clickedWithEventInfo += OnRemoveDCCToolButtonClicked;
                button.userData = dccToolInfo;
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
            MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
            if (settings.AddDCCTool(dccToolInfo)) {
                Setup(m_root);
            }
            
            
        }

        private void OnAutoDetectButtonClicked() {
            MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
            if (settings.AddInstalledDCCTools()) {
                Setup(m_root);
            }
        }


        private void OnRemoveDCCToolButtonClicked(EventBase evt) {
            DCCToolInfo dccToolInfo = GetEventButtonUserDataAs<DCCToolInfo>(evt.target);           
            if (null==dccToolInfo || string.IsNullOrEmpty(dccToolInfo.AppPath)) {
                Debug.LogWarning("[MeshSync] Failed to remove DCC Tool");
                return;
            }
            
            MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
            if (settings.RemoveDCCTool(dccToolInfo.AppPath)) {
                //Delete install info too
                string installInfoPath = DCCPluginInstallInfo.GetInstallInfoPath(dccToolInfo);
                if (File.Exists(installInfoPath)) {
                    DCCPluginInstallInfo installInfo =  FileUtility.DeserializeFromJson<DCCPluginInstallInfo>(installInfoPath);
                    installInfo.RemovePluginVersion(dccToolInfo.AppPath);
                    FileUtility.SerializeToJson(installInfo, installInfoPath);                    
                }
                
                if (!m_dccContainers.ContainsKey(dccToolInfo.AppPath)) {                    
                    Setup(m_root);
                    return;
                }

                //Remove the VisualElement container from the UI
                VisualElement container = m_dccContainers[dccToolInfo.AppPath];            
                container.parent.Remove(container);
            }
            
        }

        void OnLaunchDCCToolButtonClicked(EventBase evt) {
            DCCToolInfo dccToolInfo = GetEventButtonUserDataAs<DCCToolInfo>(evt.target);           
            if (null==dccToolInfo || string.IsNullOrEmpty(dccToolInfo.AppPath) || !File.Exists(dccToolInfo.AppPath)) {
                Debug.LogWarning("[MeshSync] Failed to launch DCC Tool");
                return;
            }
            
            DiagnosticsUtility.StartProcess(dccToolInfo.AppPath);
        }

        void OnInstallPluginButtonClicked(EventBase evt) {
            BaseDCCIntegrator integrator = GetEventButtonUserDataAs<BaseDCCIntegrator>(evt.target);           
            if (null==integrator) {
                Debug.LogWarning("[MeshSync] Failed to Install Plugin");
                return;
            }

            integrator.Integrate(() => {
                DCCToolInfo dccToolInfo = integrator.GetDCCToolInfo();
                if (!m_dccStatusLabels.ContainsKey(dccToolInfo.AppPath)) {
                    Setup(m_root);
                    return;
                }

                UpdateDCCPluginStatus(integrator, m_dccStatusLabels[dccToolInfo.AppPath]);
            });

        }
        #endregion

//----------------------------------------------------------------------------------------------------------------------        

        Texture2D LoadIcon(string iconPath) {

            if (string.IsNullOrEmpty(iconPath) || !File.Exists(iconPath)) {
                return null;
            }

            //TODO-sin: 2020-5-11: Support ico ?
            string ext = Path.GetExtension(iconPath).ToLower();
            if (ext != ".png") {
                return null;
            }
           

            byte[] fileData = File.ReadAllBytes(iconPath);
            Texture2D tex = new Texture2D(2, 2);
            tex.LoadImage(fileData, true);
            return tex;
        }


//----------------------------------------------------------------------------------------------------------------------        
        void UpdateDCCPluginStatus(BaseDCCIntegrator dccIntegrator, Label statusLabel) {
            
            DCCPluginInstallInfo installInfo = dccIntegrator.FindInstallInfo();

            const string NOT_INSTALLED = "MeshSync Plugin not installed";
            if (null == installInfo) {
                statusLabel.text = NOT_INSTALLED;                
                return;
                
            }

            DCCToolInfo dccToolInfo = dccIntegrator.GetDCCToolInfo();                
            string pluginVersion = installInfo.GetPluginVersion(dccToolInfo.AppPath);
            if (string.IsNullOrEmpty(pluginVersion)) {
                statusLabel.text = NOT_INSTALLED;
                return;
            }

            statusLabel.AddToClassList("plugin-installed");
            statusLabel.text = "MeshSync Plugin installed. Version: " + pluginVersion; 
            
        }


//----------------------------------------------------------------------------------------------------------------------
        
        static T GetEventButtonUserDataAs<T>(IEventHandler eventTarget) where T: class{
            Button button = eventTarget as Button;
            if (null == button) {
                return null;
            }

            T dccToolInfo = button.userData as T;
            return dccToolInfo;
        }
//----------------------------------------------------------------------------------------------------------------------

        private readonly Dictionary<string, Label>         m_dccStatusLabels = new Dictionary<string, Label>();
        private readonly Dictionary<string, VisualElement> m_dccContainers   = new Dictionary<string, VisualElement>();
        
        private VisualElement             m_root             = null;
        private string                    m_lastOpenedFolder = "";

    }
    
} //end namespace
