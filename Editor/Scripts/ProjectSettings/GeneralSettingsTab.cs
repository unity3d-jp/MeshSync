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
        
        VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
	        Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettings_Container")
        );
        TemplateContainer containerInstance = container.CloneTree();
        
        
        List<string> objectTypes = new List<string> {
	        MeshSyncPlayerType.SERVER.ToString(),
	        MeshSyncPlayerType.CACHE_PLAYER.ToString(),
        };	        
        
        //Add the container of this tab to root
	    VisualElement playerTypePopupContainer = containerInstance.Query<VisualElement>("PlayerTypePopupContainer").First();
        PopupField<string> playerTypePopup = new PopupField<string>(objectTypes, objectTypes[0]);
        playerTypePopup.RegisterValueChangedCallback(OnPlayerTypePopupChanged);
	    playerTypePopup.label = "Object Type";
	    playerTypePopupContainer.Add(playerTypePopup);        
        
      
	    Foldout syncSettingsFoldout = containerInstance.Query<Foldout>("SyncSettingsFoldout").First();
        VisualTreeAsset toggleTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
	        Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettingsToggleTemplate")
        );

	    //Add toggles	           
	    m_syncVisibilityToggle = AddToggle(toggleTemplate, syncSettingsFoldout, "Visibility");
	    m_syncTransformToggle = AddToggle(toggleTemplate, syncSettingsFoldout, "Transform");
	    m_syncCamerasToggle = AddToggle(toggleTemplate, syncSettingsFoldout,"Cameras");
	    m_syncLightsToggle = AddToggle(toggleTemplate, syncSettingsFoldout,"Lights");
	    m_syncMeshesToggle = AddToggle(toggleTemplate, syncSettingsFoldout,"Meshes");
	    m_updateMeshCollidersToggle = AddToggle(toggleTemplate, syncSettingsFoldout, "Update Mesh Colliders");
	    m_syncMaterialsToggle = AddToggle(toggleTemplate, syncSettingsFoldout, "Materials");
	    m_findMaterialFromAssetsToggle = AddToggle(toggleTemplate, syncSettingsFoldout, "Find Materials from Asset Database");
	    UpdatePlayerTypeUIElements(MeshSyncPlayerType.SERVER);

		//Register callbacks
	    RegisterToggleCallback(m_syncVisibilityToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncVisibility = newValue; }
		);
	    RegisterToggleCallback(m_syncTransformToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncTransform = newValue; }
	    );
	    RegisterToggleCallback(m_syncCamerasToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncCameras = newValue; }
	    );
	    RegisterToggleCallback(m_syncLightsToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncLights = newValue; }
	    );
	    RegisterToggleCallback(m_syncMeshesToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncMeshes = newValue; }
	    );
	    RegisterToggleCallback(m_updateMeshCollidersToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.UpdateMeshColliders = newValue; }
	    );
	    RegisterToggleCallback(m_syncMaterialsToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.SyncMaterials = newValue; }
	    );
	    RegisterToggleCallback(m_findMaterialFromAssetsToggle, 		    
		    (MeshSyncPlayerConfig playerConfig, bool newValue) => { playerConfig.FindMaterialFromAssets = newValue; }
	    );


	    
        root.Add(containerInstance);
    }

//----------------------------------------------------------------------------------------------------------------------	
	private Toggle AddToggle(VisualTreeAsset template, VisualElement container, string text) {

		TemplateContainer toggleContainer = template.CloneTree();
		Toggle toggle = toggleContainer.Query<Toggle>().First();
		Label label = toggleContainer.Query<Label>().First();
		label.text = text;
		container.Add(toggleContainer);
		
		return toggle;
	}
//----------------------------------------------------------------------------------------------------------------------	

	
	private void RegisterToggleCallback(Toggle toggle, 
		Action<MeshSyncPlayerConfig,bool> setter) 
	{
		
		toggle.RegisterValueChangedCallback((ChangeEvent<bool> changeEvent) => {

			MeshSyncPlayerConfig config = toggle.userData as MeshSyncPlayerConfig;
			if (null == config) {
				Debug.LogError("[MeshSync] Toggle doesn't have the correct user data");
				return;
			}
			
			setter(config, changeEvent.newValue);
			MeshSyncProjectSettings.GetInstance().SaveSettings();
		});		
	}

//----------------------------------------------------------------------------------------------------------------------	
	private void OnPlayerTypePopupChanged(ChangeEvent<string> changeEvt) {
		PopupField<string> popup = changeEvt.target as PopupField<string>;
		if (null == popup) {
			Debug.LogError("[MeshSync] ChangeEvent wasn't passed correctly");
			return;
		}

		MeshSyncPlayerType playerType = (MeshSyncPlayerType) popup.index;
		if (m_selectedPlayerType == playerType || popup.index >=(int)MeshSyncPlayerType.NUM_TYPES) {
			return;
		}

		UpdatePlayerTypeUIElements(playerType);
		changeEvt.StopPropagation();

	}

//----------------------------------------------------------------------------------------------------------------------	

	private void UpdatePlayerTypeUIElements(MeshSyncPlayerType playerType) {
		MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetInstance();
		MeshSyncPlayerConfig config = projectSettings.GetDefaultPlayerConfig(playerType);

		//values
		m_syncVisibilityToggle.SetValueWithoutNotify(config.SyncVisibility);
		m_syncTransformToggle.SetValueWithoutNotify(config.SyncTransform);
		m_syncCamerasToggle.SetValueWithoutNotify(config.SyncCameras);
		m_syncLightsToggle.SetValueWithoutNotify(config.SyncLights);
		m_syncMeshesToggle.SetValueWithoutNotify(config.SyncMeshes);
		m_updateMeshCollidersToggle.SetValueWithoutNotify(config.UpdateMeshColliders);
		m_syncMaterialsToggle.SetValueWithoutNotify(config.SyncMaterials);
		m_findMaterialFromAssetsToggle.SetValueWithoutNotify(config.FindMaterialFromAssets);
		
		//userData
		m_syncVisibilityToggle.userData = config;
		m_syncTransformToggle.userData = config;
		m_syncCamerasToggle.userData = config;
		m_syncLightsToggle.userData = config;
		m_syncMeshesToggle.userData = config;
		m_updateMeshCollidersToggle.userData = config;
		m_syncMaterialsToggle.userData = config;
		m_findMaterialFromAssetsToggle.userData = config;	
		m_selectedPlayerType = playerType;
	}
	
	
	
//----------------------------------------------------------------------------------------------------------------------
	
	private Toggle m_syncVisibilityToggle;
	private Toggle m_syncTransformToggle;
	private Toggle m_syncCamerasToggle;
	private Toggle m_syncLightsToggle;
	private Toggle m_syncMeshesToggle;
	private Toggle m_updateMeshCollidersToggle;
	private Toggle m_syncMaterialsToggle;
	private Toggle m_findMaterialFromAssetsToggle;
	private MeshSyncPlayerType m_selectedPlayerType = MeshSyncPlayerType.INVALID;

}

} //end namespace 
