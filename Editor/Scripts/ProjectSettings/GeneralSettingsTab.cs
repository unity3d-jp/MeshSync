using System;
using System.Collections.Generic;
using UnityEditor;
using System.IO;
using Unity.AnimeToolbox.Editor;
using UnityEngine;
using UnityEngine.UIElements;
using Unity.MeshSync;
using UnityEditor.UIElements;

namespace UnityEditor.MeshSync {
internal class GeneralSettingsTab : IMeshSyncSettingsTab {
	
    public GeneralSettingsTab() {
    }


//----------------------------------------------------------------------------------------------------------------------        
    public void Setup(VisualElement root) {
        root.Add(new Label("General Settings Content"));
       
        
        VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
	        Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettings_Container")
        );
        TemplateContainer containerInstance = container.CloneTree();
        
        VisualElement playerSettingsContainer = containerInstance.Query<VisualElement>("PlayerSettingsContainer").First();
        
        List<string> objectTypes = new List<string> {
	        MeshSyncObjectType.SERVER.ToString(),
	        MeshSyncObjectType.CACHE_PLAYER.ToString(),
        };	        
        
        //Add the container of this tab to root
        PopupField<string> playerSettingsPopup = new PopupField<string>(objectTypes, objectTypes[0]);
//        playerSettingsPopup.RegisterValueChangedCallback(OnToggleValueChanged);
        playerSettingsContainer.Add(playerSettingsPopup);        
        
        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetInstance();
        MeshSyncPlayerConfig serverConfig = projectSettings.GetDefaultPlayerConfig(MeshSyncObjectType.SERVER);
       
        VisualTreeAsset toggleTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
	        Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettingsToggleTemplate")
        );

	    //Add toggles	           
	    m_syncVisibilityToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_syncTransformToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_syncCamerasToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_syncLightsToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_syncMeshesToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_updateMeshCollidersToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_syncPointsToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_syncMaterialsToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    m_findMaterialFromAssetsToggle = AddToggle(toggleTemplate, playerSettingsContainer,serverConfig);
	    

		//Register callbacks
	    RegisterToggleCallback(m_syncVisibilityToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.SyncVisibility,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncVisibility = newValue; }
		);
	    RegisterToggleCallback(m_syncTransformToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.SyncTransform,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncTransform = newValue; }
	    );
	    RegisterToggleCallback(m_syncCamerasToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.SyncCameras,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncCameras = newValue; }
	    );
	    RegisterToggleCallback(m_syncLightsToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.SyncLights,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncLights = newValue; }
	    );
	    RegisterToggleCallback(m_syncMeshesToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.SyncMeshes,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncMeshes = newValue; }
	    );
	    RegisterToggleCallback(m_updateMeshCollidersToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.UpdateMeshColliders,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.UpdateMeshColliders = newValue; }
	    );
	    RegisterToggleCallback(m_syncPointsToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.SyncPoints,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncPoints = newValue; }
	    );
	    RegisterToggleCallback(m_syncMaterialsToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.SyncMaterials,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncMaterials = newValue; }
	    );
	    RegisterToggleCallback(m_findMaterialFromAssetsToggle, 		    
		    (MeshSyncPlayerConfig playerConfig) => playerConfig.FindMaterialFromAssets,
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.FindMaterialFromAssets = newValue; }
	    );


	    
        root.Add(containerInstance);
    }

//----------------------------------------------------------------------------------------------------------------------	
	private Toggle AddToggle(VisualTreeAsset template, VisualElement container, MeshSyncPlayerConfig config) {

		TemplateContainer toggleContainer = template.CloneTree();
		Toggle toggle = toggleContainer.Query<Toggle>().First();
		toggle.userData = config;

		container.Add(toggleContainer);
		
		return toggle;
	}
//----------------------------------------------------------------------------------------------------------------------	

	
	private void RegisterToggleCallback(Toggle toggle, 
		Func<MeshSyncPlayerConfig, bool> getter,
		Action<MeshSyncPlayerConfig,bool> setter) 
	{
		MeshSyncPlayerConfig config = toggle.userData as MeshSyncPlayerConfig;
		if (null == config) {
			Debug.LogError("[MeshSync] Toggle doesn't have the correct user data");
			return;
		}
		
		toggle.value = getter(config);		
		toggle.RegisterValueChangedCallback((changeEvent) => {
			setter(config, changeEvent.newValue);
			MeshSyncProjectSettings.GetInstance().SaveSettings();
		});		
	}

	
//----------------------------------------------------------------------------------------------------------------------
	
	private Toggle m_syncVisibilityToggle;
	private Toggle m_syncTransformToggle;
	private Toggle m_syncCamerasToggle;
	private Toggle m_syncLightsToggle;
	private Toggle m_syncMeshesToggle;
	private Toggle m_updateMeshCollidersToggle;
	private Toggle m_syncPointsToggle;
	private Toggle m_syncMaterialsToggle;
	private Toggle m_findMaterialFromAssetsToggle;
	

}

} //end namespace 
