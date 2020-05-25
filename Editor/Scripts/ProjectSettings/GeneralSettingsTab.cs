using System;
using System.Collections.Generic;
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
	    m_playerConfigUIElements = new List<VisualElement>();
        VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
	        Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettings_Container")
        );
        TemplateContainer containerInstance = container.CloneTree();
        
        
        List<string> objectTypes = new List<string> {
	        MeshSyncPlayerType.SERVER.ToString(),
	        MeshSyncPlayerType.CACHE_PLAYER.ToString(),
        };	        
        
        //Add the parent of this tab to root
	    VisualElement playerTypePopupContainer = containerInstance.Query<VisualElement>("PlayerTypePopupContainer").First();
        PopupField<string> playerTypePopup = new PopupField<string>(objectTypes, objectTypes[0]);
        playerTypePopup.RegisterValueChangedCallback(OnPlayerTypePopupChanged);
	    playerTypePopup.label = "Settings for object type";
	    playerTypePopupContainer.Add(playerTypePopup);        

	    //Templates
	    VisualTreeAsset fieldTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
		    Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettingsFieldTemplate")
	    );
      
	    Foldout syncSettingsFoldout = containerInstance.Query<Foldout>("SyncSettingsFoldout").First();

	    //Sync	           
	    m_syncVisibilityToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, "Visibility",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncVisibility = newValue; }
	    );
	    m_syncTransformToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, "Transform",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncTransform = newValue; }
	    );
	    m_syncCamerasToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout,"Cameras",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncCameras = newValue; }
	    );
	    m_syncLightsToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout,"Lights",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncLights = newValue; }
	    );
	    m_syncMeshesToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout,"Meshes",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncMeshes = newValue; }
	    );
	    m_updateMeshCollidersToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, 
		    "Update Mesh Colliders",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.UpdateMeshColliders = newValue; }
	    );
	    m_syncMaterialsToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, "Materials",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncMaterials = newValue; }
	    );
	    m_findMaterialFromAssetsToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, 
		    "Find Materials from Asset Database",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.FindMaterialFromAssets = newValue; }
	    );

		//import
	    Foldout importSettingsFoldout = containerInstance.Query<Foldout>("ImportSettingsFoldout").First();

	    m_animationInterpolationPopup = AddPopupField(fieldTemplate, importSettingsFoldout, 
		    "Animation Interpolation", m_animationInterpolationEnums,
		    (MeshSyncPlayerConfig config, int newValue) => { config.AnimationInterpolation = newValue; }
	    );
	    m_keyframeReductionToggle = AddField<Toggle,bool>(fieldTemplate, importSettingsFoldout, "Keyframe Reduction",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.KeyframeReduction = newValue; }
	    );
	    m_reductionThresholdField = AddField<FloatField, float>(fieldTemplate, importSettingsFoldout, 
		    "Reduction Threshold",
		    (MeshSyncPlayerConfig config, float newValue) => { config.ReductionThreshold = newValue; }
		);
	    m_reductionEraseFlatCurves = AddField<Toggle,bool>(fieldTemplate, importSettingsFoldout, 
		    "Reduction Erase Flat Curves",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.ReductionEraseFlatCurves = newValue; }
		);
	    m_zUpCorrectionPopup = AddPopupField(fieldTemplate, importSettingsFoldout, 
		    "Z-Up correction", m_zUpCorrectionEnums,
		    (MeshSyncPlayerConfig config, int newValue) => { config.ZUpCorrection = newValue; }
	    );
	    
	    //Misc 
	    Foldout miscSettingsFoldout = containerInstance.Query<Foldout>("MiscSettingsFoldout").First();
	    m_syncMaterialListToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    "Sync Material List",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncMaterialList = newValue; }
	    );
	    m_progressiveDisplayToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    "Progressive Display",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.ProgressiveDisplay = newValue; }
	    );
	    m_loggingToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    "Logging",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.Logging = newValue; }
	    );
	    m_profilingToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    "Profiling",
		    (MeshSyncPlayerConfig config, bool newValue) => { config.Profiling = newValue; }
	    );
	    
	    //Animation Tweak
	    Foldout animationTweakSettingsFoldout = containerInstance.Query<Foldout>("AnimationTweakSettingsFoldout").First();
	    m_animationTweakTimeScaleField = AddField<FloatField, float>(fieldTemplate, animationTweakSettingsFoldout, 
		    "Time Scale",
		    (MeshSyncPlayerConfig config, float newValue) => {
			    config.GetAnimationTweakSettings().TimeScale = newValue;
		    }
	    );
	    m_animationTweakTimeOffsetField = AddField<FloatField, float>(fieldTemplate, animationTweakSettingsFoldout, 
		    "Time Offset",
		    (MeshSyncPlayerConfig config, float newValue) => {
			    config.GetAnimationTweakSettings().TimeOffset = newValue;
		    }
	    );
	    m_animationTweakDropStepField = AddField<IntegerField, int>(fieldTemplate, animationTweakSettingsFoldout, 
		    "Drop Step",
		    (MeshSyncPlayerConfig config, int newValue) => {
			    config.GetAnimationTweakSettings().DropStep = newValue;
		    }
	    );
	    m_animationTweakReductionThresholdField = AddField<FloatField, float>(fieldTemplate, animationTweakSettingsFoldout, 
		    "Reduction Threshold",
		    (MeshSyncPlayerConfig config, float newValue) => {
			    config.GetAnimationTweakSettings().ReductionThreshold = newValue;
		    }
	    );
	    m_animationTweakEraseFlatCurvesToggle = AddField<Toggle, bool>(fieldTemplate, animationTweakSettingsFoldout, 
		    "Erase Flat Curves",
		    (MeshSyncPlayerConfig config, bool newValue) => {
			    config.GetAnimationTweakSettings().EraseFlatCurves = newValue;
		    }
	    );
	    	       
	    UpdatePlayerConfigUIElements(MeshSyncPlayerType.SERVER);
	    
        root.Add(containerInstance);
    }

//----------------------------------------------------------------------------------------------------------------------	

	//Support Toggle, FloatField, etc
	private F AddField<F,V>(VisualTreeAsset template, VisualElement parent, string text,
		Action<MeshSyncPlayerConfig,V> onValueChanged) where F: VisualElement,INotifyValueChanged<V>, new()  
	{

		TemplateContainer templateInstance = template.CloneTree();
		VisualElement fieldContainer = templateInstance.Query<VisualElement>("FieldContainer").First();
//		F field = templateInstance.Query<F>().First();
		Label label = templateInstance.Query<Label>().First();
		label.text = text;
		
		F field = new F();
		field.AddToClassList("general-settings-field");
		field.RegisterValueChangedCallback((ChangeEvent<V> changeEvent) => {

			MeshSyncPlayerConfig config = field.userData as MeshSyncPlayerConfig;
			if (null == config) {
				Debug.LogError("[MeshSync] Field doesn't have the correct user data");
				return;
			}
			
			onValueChanged(config, changeEvent.newValue);
			MeshSyncProjectSettings.GetOrCreateSettings().SaveSettings();
		});		
		
		fieldContainer.Add(field);
		parent.Add(templateInstance);
		m_playerConfigUIElements.Add(field);		
		return field;
	}
	
//----------------------------------------------------------------------------------------------------------------------	
	private PopupField<T> AddPopupField<T>(VisualTreeAsset template, VisualElement parent, string text,
		List<T> options, Action<MeshSyncPlayerConfig,int> onValueChanged) 
	{

		TemplateContainer templateInstance = template.CloneTree();
		VisualElement fieldContainer = templateInstance.Query<VisualElement>("FieldContainer").First();
		PopupField<T> popupField = new PopupField<T>(options,options[0]);
		popupField.AddToClassList("general-settings-field");
		
		Label label = templateInstance.Query<Label>().First();
		label.text = text;
		popupField.RegisterValueChangedCallback( ( ChangeEvent<T> changeEvent)  => {
		
			MeshSyncPlayerConfig config = popupField.userData as MeshSyncPlayerConfig;
			if (null == config) {
				Debug.LogError("[MeshSync] Toggle doesn't have the correct user data");
				return;
			}
			
			onValueChanged(config, popupField.index);
			MeshSyncProjectSettings.GetOrCreateSettings().SaveSettings();
		});
				
		fieldContainer.Add(popupField);
		parent.Add(templateInstance);
		m_playerConfigUIElements.Add(popupField);		
		return popupField;
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

		UpdatePlayerConfigUIElements(playerType);
		changeEvt.StopPropagation();

	}

//----------------------------------------------------------------------------------------------------------------------	

	private void UpdatePlayerConfigUIElements(MeshSyncPlayerType playerType) {
		MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();
		MeshSyncPlayerConfig config = projectSettings.GetDefaultPlayerConfig(playerType);

		//sync
		m_syncVisibilityToggle.SetValueWithoutNotify(config.SyncVisibility);
		m_syncTransformToggle.SetValueWithoutNotify(config.SyncTransform);
		m_syncCamerasToggle.SetValueWithoutNotify(config.SyncCameras);
		m_syncLightsToggle.SetValueWithoutNotify(config.SyncLights);
		m_syncMeshesToggle.SetValueWithoutNotify(config.SyncMeshes);
		m_updateMeshCollidersToggle.SetValueWithoutNotify(config.UpdateMeshColliders);
		m_syncMaterialsToggle.SetValueWithoutNotify(config.SyncMaterials);
		m_findMaterialFromAssetsToggle.SetValueWithoutNotify(config.FindMaterialFromAssets);

		//Import
		m_animationInterpolationPopup.SetValueWithoutNotify(m_animationInterpolationEnums[config.AnimationInterpolation]);
		m_keyframeReductionToggle.SetValueWithoutNotify(config.KeyframeReduction);
		m_reductionThresholdField.SetValueWithoutNotify(config.ReductionThreshold);
		m_reductionEraseFlatCurves.SetValueWithoutNotify(config.ReductionEraseFlatCurves);
		m_zUpCorrectionPopup.SetValueWithoutNotify(m_zUpCorrectionEnums[config.ZUpCorrection]);

		//Misc
		m_syncMaterialListToggle.SetValueWithoutNotify(config.SyncMaterialList);
		m_progressiveDisplayToggle.SetValueWithoutNotify(config.ProgressiveDisplay);
		m_loggingToggle.SetValueWithoutNotify(config.Logging);
		m_profilingToggle.SetValueWithoutNotify(config.Profiling);

		//Animation Tweak
		AnimationTweakSettings animationTweakSettings = config.GetAnimationTweakSettings();
		m_animationTweakTimeScaleField.SetValueWithoutNotify(animationTweakSettings.TimeScale);
		m_animationTweakTimeOffsetField.SetValueWithoutNotify(animationTweakSettings.TimeOffset);
		m_animationTweakDropStepField.SetValueWithoutNotify(animationTweakSettings.DropStep);
		m_animationTweakReductionThresholdField.SetValueWithoutNotify(animationTweakSettings.ReductionThreshold);
		m_animationTweakEraseFlatCurvesToggle.SetValueWithoutNotify(animationTweakSettings.EraseFlatCurves);
	
		//userData
		foreach (VisualElement uiElement in m_playerConfigUIElements) {
			uiElement.userData = config;
		}
		
		
		m_selectedPlayerType = playerType;
	}


//----------------------------------------------------------------------------------------------------------------------
	
	//Sync Settings
	private Toggle m_syncVisibilityToggle;
	private Toggle m_syncTransformToggle;
	private Toggle m_syncCamerasToggle;
	private Toggle m_syncLightsToggle;
	private Toggle m_syncMeshesToggle;
	private Toggle m_updateMeshCollidersToggle;
	private Toggle m_syncMaterialsToggle;
	private Toggle m_findMaterialFromAssetsToggle;
	
	//Import Settings
	private PopupField<string> m_animationInterpolationPopup;
	private Toggle m_keyframeReductionToggle;
	private FloatField m_reductionThresholdField;
	private Toggle m_reductionEraseFlatCurves;
	private PopupField<string> m_zUpCorrectionPopup;
	
	//Misc Settings
	private Toggle m_syncMaterialListToggle;
	private Toggle m_progressiveDisplayToggle;
	private Toggle m_loggingToggle;
	private Toggle m_profilingToggle;
	
	//AnimationTweak Settings
	public FloatField   m_animationTweakTimeScaleField;
	public FloatField   m_animationTweakTimeOffsetField;
	public IntegerField m_animationTweakDropStepField;
	public FloatField   m_animationTweakReductionThresholdField;
	public Toggle       m_animationTweakEraseFlatCurvesToggle;
	
	
	private MeshSyncPlayerType m_selectedPlayerType = MeshSyncPlayerType.INVALID;
	private List<VisualElement> m_playerConfigUIElements;

	private readonly List<string> m_animationInterpolationEnums = new List<string>(Enum.GetNames( typeof( InterpolationMode )));
	private readonly List<string> m_zUpCorrectionEnums = new List<string>(Enum.GetNames( typeof( ZUpCorrectionMode )));
	
}

} //end namespace 
