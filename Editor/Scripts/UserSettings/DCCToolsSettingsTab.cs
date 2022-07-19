using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;
using Unity.EditorCoroutines.Editor;
using UnityEditor.PackageManager;
using Debug = UnityEngine.Debug;

namespace Unity.MeshSync.Editor {
internal class DCCToolsSettingsTab : IMeshSyncSettingsTab {
    
    private IDccToolVersionVerifier m_blenderVersionVerifier = new BlenderVersionVerifier();
    
    public DCCToolsSettingsTab() {
        
        
    }

//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        SetupInternal(root);
        OnCheckPluginUpdatesButtonClicked();
    }

//----------------------------------------------------------------------------------------------------------------------        
    private void SetupInternal(VisualElement root) {

        m_dccStatusLabels.Clear();
        m_dccContainers.Clear();
        
        m_root = root;
        m_root.Clear();
        m_installPluginButtons.Clear();

        VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
            MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_CONTAINER_PATH
        );
        
        VisualTreeAsset dccToolInfoTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
            MeshSyncEditorConstants.DCC_TOOL_INFO_TEMPLATE_PATH
        );

        TemplateContainer containerInstance = container.CloneTree();
        ScrollView scrollView = containerInstance.Query<ScrollView>().First();

       
        //Buttons
        Button autoDetectDCCButton = containerInstance.Query<Button>("AutoDetectDCCButton").First();
        autoDetectDCCButton.clickable.clicked += OnAutoDetectDCCButtonClicked;
        m_checkPluginUpdatesButton = containerInstance.Query<Button>("ChecksPluginUpdatesButton").First();
        m_checkPluginUpdatesButton.clickable.clicked += OnCheckPluginUpdatesButtonClicked;
        Button addDCCToolButton = containerInstance.Query<Button>("AddDCCToolButton").First();
        addDCCToolButton.userData                       =  scrollView;
        addDCCToolButton.clickable.clickedWithEventInfo += OnAddDCCToolButtonClicked;

        Button showBlenderZips = containerInstance.Query<Button>("ShowDCCZips").First();
        showBlenderZips.clickable.clicked += OnShowDCCZipsClicked;

        //Label
        m_footerStatusLabel = containerInstance.Query<Label>("FooterStatusLabel").First();

        //Add detected DCCTools to ScrollView
        MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
        foreach (KeyValuePair<string, DCCToolInfo> dccToolInfo in settings.GetDCCToolInfos()) {
            AddDCCToolSettingsContainer(dccToolInfo.Value, scrollView, dccToolInfoTemplate);                
        }            
        
        //Add the container of this tab to root
        root.Add(containerInstance);
    }

    private void OnShowDCCZipsClicked() {
        var path = Application.dataPath;
        path = path.Replace("/Assets", "");
        path = Path.Combine(path, "Packages","com.unity.meshsync.dcc-plugins", "Editor", "Plugins~");
        path = Path.GetFullPath(path);
        
        switch (Application.platform) {
            case RuntimePlatform.OSXEditor:
                Process.Start("open", path);
                break;
            case RuntimePlatform.WindowsEditor:
                Process.Start("explorer.exe", $"\"{path}\"");
                break;
            default:
                Debug.LogErrorFormat("[MeshSync] Show zip files: {0} not supported", Application.platform);
                break;
        }
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
        statusLabel.userData = integrator;
        UpdateDCCPluginStatusLabel(statusLabel);
        
        m_dccStatusLabels[dccToolInfo.AppPath] = statusLabel;
        m_dccContainers[dccToolInfo.AppPath]   = container; 
        
        SetupButtons(dccToolInfo, container, integrator, IsToolSupported(dccToolInfo));


        top.Add(container);
    }

    private bool IsToolSupported(DCCToolInfo tool) {
        //Buttons
        switch (tool.Type) {
            case DCCToolType.BLENDER:
                return m_blenderVersionVerifier.IsSupported(tool.DCCToolVersion);
            default:
                //TODO handle other DCC tools
                return true;
        }
    }

    private string GetLatestSupporterToolVersion(DCCToolInfo tool) {
        switch (tool.Type) {
            case DCCToolType.BLENDER:
                return m_blenderVersionVerifier.LatestSupportedVersion;
            default:
                return "";
        }
    }


    private void SetupButtons(DCCToolInfo dccToolInfo, TemplateContainer container,
        BaseDCCIntegrator integrator, bool supportedVersion) {
        {
            Button button = container.Query<Button>("LaunchDCCToolButton").First();
            button.clickable.clickedWithEventInfo += OnLaunchDCCToolButtonClicked;
            button.userData                       =  dccToolInfo;
            button.SetEnabled(supportedVersion);
        }
        {
            Button button = container.Query<Button>("InstallPluginButton").First();
            button.clickable.clickedWithEventInfo += OnInstallPluginButtonClicked;
            button.userData                       =  integrator;
            button.SetEnabled(m_checkPluginUpdatesButton.enabledSelf);
            m_installPluginButtons.Add(button);
            button.SetEnabled(supportedVersion);
        }
        {
            Button button = container.Query<Button>("RemoveDCCToolButton").First();
            button.clickable.clickedWithEventInfo += OnRemoveDCCToolButtonClicked;
            button.userData                       =  dccToolInfo;
            button.SetEnabled(true);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------        

    #region Button callbacks

    private const string addDCCToolLabel = "Add DCC Tool";
    
    string GetFilePath() {
        var path = EditorUtility.OpenFilePanel(addDCCToolLabel, m_lastOpenedFolder, "");
        
        if (!string.IsNullOrEmpty(path)) {
            m_lastOpenedFolder = Path.GetDirectoryName(path);
        }
        
        return path;
    }

    string GetFolderPath() {
        var path = EditorUtility.OpenFolderPanel(addDCCToolLabel, m_lastOpenedFolder, "");
        
        if (!string.IsNullOrEmpty(path)) {
            m_lastOpenedFolder = path;
        }

        return path;
    }
    
    void OnAddDCCToolButtonClicked(EventBase evt) {
        string path = null;
        if (Application.platform == RuntimePlatform.OSXEditor) {
            path = GetFilePath();
        }
        else {
            path = GetFolderPath();
        }
        
        if (string.IsNullOrEmpty(path)) {
            return;
        }

        //Find the path to the actual app
        DCCToolType lastDCCToolType = DCCToolType.AUTODESK_MAYA;
        DCCToolInfo dccToolInfo = null;
        for (int i = 0; i < (int) (DCCToolType.NUM_DCC_TOOL_TYPES) && null==dccToolInfo; ++i) {
            lastDCCToolType = (DCCToolType) (i);
            dccToolInfo = DCCFinderUtility.FindDCCToolInDirectory(lastDCCToolType, null, path);
        }

        if (null==dccToolInfo) {
            EditorUtility.DisplayDialog("MeshSync Project Settings", "No DCC Tool is detected", "Ok");
            return;
        }
        
        MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
        if (settings.AddDCCTool(dccToolInfo)) {
            //Add to ScrollView
            VisualTreeAsset dccToolInfoTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
                MeshSyncEditorConstants.DCC_TOOL_INFO_TEMPLATE_PATH
            );
            ScrollView scrollView = GetEventButtonUserDataAs<ScrollView>(evt.target);
            Assert.IsNotNull(scrollView);
            AddDCCToolSettingsContainer(dccToolInfo, scrollView, dccToolInfoTemplate);                
        }
    }

    private void OnAutoDetectDCCButtonClicked() {
        MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
        if (settings.AddInstalledDCCTools()) {
            SetupInternal(m_root);
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
            
            if (!m_dccContainers.TryGetValue(dccToolInfo.AppPath, out VisualElement container)) {                    
                SetupInternal(m_root);
                return;
            }
            
            container.parent.Remove(container); //Remove the VisualElement container from the UI
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

        if (null == m_latestCompatibleDCCPluginVersion) {
            EditorUtility.DisplayDialog("MeshSync",
                $"DCC Plugin compatible with MeshSync@{MeshSyncEditorConstants.GetPluginVersion()} is not found", 
                "Ok"
            );
            return;
        }            
        
        BaseDCCIntegrator integrator = GetEventButtonUserDataAs<BaseDCCIntegrator>(evt.target);           
        if (null==integrator) {
            Debug.LogWarning("[MeshSync] Failed to Install Plugin");
            return;
        }

        integrator.Integrate(m_latestCompatibleDCCPluginVersion.ToString(), () => {
            DCCToolInfo dccToolInfo = integrator.GetDCCToolInfo();                
            if (!m_dccStatusLabels.TryGetValue(dccToolInfo.AppPath, out Label statusLabel)) {
                SetupInternal(m_root);
                return;
            }

            UpdateDCCPluginStatusLabel(statusLabel);
        });

    }
    #endregion
    
//----------------------------------------------------------------------------------------------------------------------        
    
    #region CheckPluginUpdates Button callback
    private void OnCheckPluginUpdatesButtonClicked() {
        m_checkPluginUpdatesButton.SetEnabled(false);

        //Disable installing plugin while we are checking for updates
        foreach (Button installPluginButton in m_installPluginButtons) {
            installPluginButton.SetEnabled(false);
        }           
        
        m_updateFooterStatusFinished = false;
        EditorCoroutineUtility.StartCoroutineOwnerless(UpdateFooterStatusLabel("Checking", FinalizeCheckPluginUpdates));
        
        PackageRequestJobManager.CreateSearchRequest("com.unity.meshsync.dcc-plugins", /*offline=*/ false, (packageInfo) => {
            //just in case
            if (packageInfo.Result.Length <= 0) {
                Debug.LogError("[MeshSync] Failed to check DCC Plugin updates");
                m_updateFooterStatusFinished = true;
                return;
            }

            //Update status labels
            UpdateLatestCompatibleDCCPlugin(packageInfo.Result[0].versions);
            foreach (KeyValuePair<string, Label> kv in m_dccStatusLabels) {
                UpdateDCCPluginStatusLabel(kv.Value);
            }
            
            m_updateFooterStatusFinished = true;
        }, (req)=> {                
            m_updateFooterStatusFinished = true;
        });            
    }

    void UpdateLatestCompatibleDCCPlugin(VersionsInfo versionsInfo) {
        PackageVersion pluginVer = MeshSyncEditorConstants.GetPluginVersion();
        
        foreach (string dccPluginVerStr in versionsInfo.all) {

            //Skip incompatible versions
            if (!IsPackageVersionCompatible(dccPluginVerStr, pluginVer, out PackageVersion dccPluginVer))
                continue;
            
            if (null == m_latestCompatibleDCCPluginVersion 
                || dccPluginVer.GetPatch() > m_latestCompatibleDCCPluginVersion.GetPatch()) 
            {
                m_latestCompatibleDCCPluginVersion = dccPluginVer;
            }
        }
    }
    

    private void FinalizeCheckPluginUpdates() {
        m_footerStatusLabel.text = "";
        m_checkPluginUpdatesButton.SetEnabled(true);  
        
        foreach (Button installPluginButton in m_installPluginButtons) {
            
            var integrator  = installPluginButton.userData as BaseDCCIntegrator;
            var isSupported = true;
            if (integrator != null) {
                var info        = integrator.GetDCCToolInfo();
                isSupported = IsToolSupported(info);
            }

            installPluginButton.SetEnabled(isSupported);

        }           
        
    }

    private IEnumerator UpdateFooterStatusLabel(string reqStatusText, System.Action onFinished) {            
        const int MAX_PREFIX_LENGTH = 16;
        const int MAX_STATUS_LENGTH = 32;
        string    mainStatusText    = reqStatusText;

        if (mainStatusText.Length >= MAX_PREFIX_LENGTH) {
            mainStatusText = reqStatusText.Substring(0, MAX_PREFIX_LENGTH);
        }

        string labelText = mainStatusText;
        while (!m_updateFooterStatusFinished) {
            labelText += ".";
            if (labelText.Length > MAX_STATUS_LENGTH) {
                labelText = mainStatusText;
            }
            
            m_footerStatusLabel.text =  labelText;
            yield return new EditorWaitForSeconds(1);
        }

        onFinished();
        yield return null;
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
    void UpdateDCCPluginStatusLabel(Label statusLabel) {
        
        BaseDCCIntegrator dccIntegrator = statusLabel.userData as BaseDCCIntegrator;
        Assert.IsNotNull(dccIntegrator);

        var toolInfo = dccIntegrator.GetDCCToolInfo();
        if(!IsToolSupported(toolInfo)) {
            toolInfo.GetDescription();
            string NOT_SUPPORTED = $"{toolInfo.GetDescription()} is not supported" +
                $"- latest supported version is {GetLatestSupporterToolVersion(toolInfo)}";

            statusLabel.text = NOT_SUPPORTED;
            return;
        }
        
        DCCPluginInstallInfo installInfo = dccIntegrator.FindInstallInfo();

        const string NOT_INSTALLED = "MeshSync Plugin not installed";
        if (null == installInfo) {
            statusLabel.text = NOT_INSTALLED;                
            return;                
        }

        DCCToolInfo dccToolInfo = dccIntegrator.GetDCCToolInfo();            
        string installedPluginVersionStr = installInfo.GetPluginVersion(dccToolInfo.AppPath);
        if (string.IsNullOrEmpty(installedPluginVersionStr)) {
            statusLabel.text = NOT_INSTALLED;
            return;
        }
        
        //Remove all known classes
        const string PLUGIN_INCOMPATIBLE_CLASS  = "plugin-incompatible";
        const string PLUGIN_INSTALLED_OLD_CLASS = "plugin-installed-old";
        const string PLUGIN_INSTALLED_CLASS     = "plugin-installed";
        statusLabel.RemoveFromClassList(PLUGIN_INCOMPATIBLE_CLASS);
        statusLabel.RemoveFromClassList(PLUGIN_INSTALLED_CLASS);
        statusLabel.RemoveFromClassList(PLUGIN_INSTALLED_OLD_CLASS);

        PackageVersion pluginVer = MeshSyncEditorConstants.GetPluginVersion();
        
        //The DCC Plugin is installed, and we need to check if it's compatible with this version of MeshSync
        if (!IsPackageVersionCompatible(installedPluginVersionStr, pluginVer, out PackageVersion installedPluginVersion)) {
            statusLabel.AddToClassList(PLUGIN_INCOMPATIBLE_CLASS);
            statusLabel.text = "Installed MeshSync Plugin is incompatible. Version: " + installedPluginVersionStr; 
            return;
        }
        
        
        //Check if we have newer compatible DCCPlugin
        if (null!= m_latestCompatibleDCCPluginVersion 
            && installedPluginVersion.GetPatch() < m_latestCompatibleDCCPluginVersion.GetPatch()) 
        {                
            statusLabel.AddToClassList(PLUGIN_INSTALLED_OLD_CLASS);
            statusLabel.text = $"Plugin {installedPluginVersionStr} installed. " +
                $"({m_latestCompatibleDCCPluginVersion} is available)";
            return;
        } 

        statusLabel.AddToClassList(PLUGIN_INSTALLED_CLASS);
        statusLabel.text = $"Plugin {installedPluginVersionStr} installed"; 
        
    }

    internal static bool IsPackageVersionCompatible(string ver0Str, PackageVersion ver1, out PackageVersion ver0) {

        bool parsed = PackageVersion.TryParse(ver0Str, out ver0);
        if (!parsed)
            return false;
        
        return ver0.GetMajor() == ver1.GetMajor() && ver0.GetMinor() == ver1.GetMinor();
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
    private readonly List<Button>                      m_installPluginButtons = new List<Button>();

    private static PackageVersion m_latestCompatibleDCCPluginVersion = null;
    private Button         m_checkPluginUpdatesButton = null;
    private Label          m_footerStatusLabel        = null;

    private bool m_updateFooterStatusFinished = false;
    
   
    private VisualElement             m_root             = null;
    private string                    m_lastOpenedFolder = "";

}
    
} //end namespace
