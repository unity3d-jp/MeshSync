using System;
using System.Collections.Generic;
using System.IO;
using Unity.AnimeToolbox.Editor;
using UnityEngine;
using UnityEngine.UIElements;
using Unity.MeshSync;
using UnityEditor;
using UnityEditor.UIElements;

namespace Unity.MeshSync.Editor {
internal class GeneralSettingsTab : IMeshSyncSettingsTab {

	internal class Contents {

		public static readonly GUIContent ServerPort = EditorGUIUtility.TrTextContent("Server Port");
		public static readonly GUIContent AllowPublicAccess = EditorGUIUtility.TrTextContent("Allow public access");
		public static readonly GUIContent Visibility = EditorGUIUtility.TrTextContent("Visibility");
		public static readonly GUIContent Transform  = EditorGUIUtility.TrTextContent("Transform");
		public static readonly GUIContent Cameras = EditorGUIUtility.TrTextContent("Cameras");
		public static readonly GUIContent Lights  = EditorGUIUtility.TrTextContent("Lights");
		public static readonly GUIContent Meshes = EditorGUIUtility.TrTextContent("Meshes");
		public static readonly GUIContent UpdateMeshColliders = EditorGUIUtility.TrTextContent("Update Mesh Colliders");
		public static readonly GUIContent Materials = EditorGUIUtility.TrTextContent("Materials");
		public static readonly GUIContent FindMaterialsFromAssetDatabase = EditorGUIUtility.TrTextContent("Find Materials from Asset Database");

		public static readonly GUIContent AnimationInterpolation = EditorGUIUtility.TrTextContent("Animation Interpolation");
		public static readonly GUIContent KeyframeReduction  = EditorGUIUtility.TrTextContent("Keyframe Reduction");
		public static readonly GUIContent ReductionThreshold = EditorGUIUtility.TrTextContent("Reduction Threshold");
		public static readonly GUIContent ReductionEraseFlatCurves = EditorGUIUtility.TrTextContent("Reduction Erase Flat Curves");
		public static readonly GUIContent ZUpCorrection = EditorGUIUtility.TrTextContent("Z-Up correction");

		
		public static readonly GUIContent SyncMaterialList = EditorGUIUtility.TrTextContent("Sync Material List");
		public static readonly GUIContent ProgressiveDisplay = EditorGUIUtility.TrTextContent("Progressive Display");
		public static readonly GUIContent Logging = EditorGUIUtility.TrTextContent("Logging");
		public static readonly GUIContent Profiling = EditorGUIUtility.TrTextContent("Profiling");

		public static readonly GUIContent TweakTimeScale = EditorGUIUtility.TrTextContent("Time Scale");
		public static readonly GUIContent TweakTimeOffset = EditorGUIUtility.TrTextContent("Time Offset");
		public static readonly GUIContent TweakDropStep = EditorGUIUtility.TrTextContent("Drop Step");
		public static readonly GUIContent TweakReductionThreshold = EditorGUIUtility.TrTextContent("Reduction Threshold");
		public static readonly GUIContent TweakEraseFlatCurves = EditorGUIUtility.TrTextContent("Erase Flat Curves");
		
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
        
	    //Templates
	    VisualTreeAsset fieldTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
		    Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettingsFieldTemplate")
	    );

	    //Add server port
	    MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
	    VisualElement  headerContainer = containerInstance.Query<VisualElement>("HeaderContainer").First();
	    m_serverPortField = new IntegerField("Server Port");
	    m_serverPortField.SetValueWithoutNotify(runtimeSettings.GetDefaultServerPort());
	    
	    m_serverPortField.RegisterValueChangedCallback((ChangeEvent<int> evt) => {
		    MeshSyncRuntimeSettings settings = MeshSyncRuntimeSettings.GetOrCreateSettings();
		    settings.SetDefaultServerPort((ushort) evt.newValue);
		    settings.SaveSettings();
	    });
	    headerContainer.Add(m_serverPortField);

	    m_allowPublicAccessToggle = new Toggle(Contents.AllowPublicAccess.text);
	    m_allowPublicAccessToggle.tooltip = Contents.AllowPublicAccess.tooltip;
	    m_allowPublicAccessToggle.SetValueWithoutNotify(runtimeSettings.GetPublicAccess());
	    m_allowPublicAccessToggle.RegisterValueChangedCallback( (ChangeEvent<bool> evt) => {
		    MeshSyncRuntimeSettings settings = MeshSyncRuntimeSettings.GetOrCreateSettings();
		    settings.SetPublicAccess(evt.newValue);
		    settings.SaveSettings();
	    });
	    headerContainer.Add(m_allowPublicAccessToggle);
	        
	    
        //Add playerType popup
	    VisualElement playerTypePopupContainer = containerInstance.Query<VisualElement>("PlayerTypePopupContainer").First();
        PopupField<string> playerTypePopup = new PopupField<string>("Settings for object type",objectTypes, objectTypes[0]);
        playerTypePopup.RegisterValueChangedCallback(OnPlayerTypePopupChanged);
	    playerTypePopupContainer.Add(playerTypePopup);        

      
	    Foldout syncSettingsFoldout = containerInstance.Query<Foldout>("SyncSettingsFoldout").First();

	    //Sync	           
	    m_syncVisibilityToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, Contents.Visibility,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncVisibility = newValue; }
	    );
	    m_syncTransformToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, Contents.Transform,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncTransform = newValue; }
	    );
	    m_syncCamerasToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, Contents.Cameras,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncCameras = newValue; }
	    );
	    m_syncLightsToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, Contents.Lights,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncLights = newValue; }
	    );
	    m_syncMeshesToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, Contents.Meshes,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncMeshes = newValue; }
	    );
	    m_updateMeshCollidersToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, 
		    Contents.UpdateMeshColliders,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.UpdateMeshColliders = newValue; }
	    );
	    m_syncMaterialsToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, Contents.Materials,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncMaterials = newValue; }
	    );
	    m_findMaterialFromAssetsToggle = AddField<Toggle,bool>(fieldTemplate, syncSettingsFoldout, 
		    Contents.FindMaterialsFromAssetDatabase,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.FindMaterialFromAssets = newValue; }
	    );

		//import
	    Foldout importSettingsFoldout = containerInstance.Query<Foldout>("ImportSettingsFoldout").First();

	    m_animationInterpolationPopup = AddPopupField(fieldTemplate, importSettingsFoldout, 
		    Contents.AnimationInterpolation, m_animationInterpolationEnums,
		    (MeshSyncPlayerConfig config, int newValue) => { config.AnimationInterpolation = newValue; }
	    );
	    m_keyframeReductionToggle = AddField<Toggle,bool>(fieldTemplate, importSettingsFoldout, 
		    Contents.KeyframeReduction,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.KeyframeReduction = newValue; }
	    );
	    m_reductionThresholdField = AddField<FloatField, float>(fieldTemplate, importSettingsFoldout, 
		    Contents.ReductionThreshold,
		    (MeshSyncPlayerConfig config, float newValue) => { config.ReductionThreshold = newValue; }
		);
	    m_reductionEraseFlatCurves = AddField<Toggle,bool>(fieldTemplate, importSettingsFoldout, 
		    Contents.ReductionEraseFlatCurves,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.ReductionEraseFlatCurves = newValue; }
		);
	    m_zUpCorrectionPopup = AddPopupField(fieldTemplate, importSettingsFoldout, 
		    Contents.ZUpCorrection, m_zUpCorrectionEnums,
		    (MeshSyncPlayerConfig config, int newValue) => { config.ZUpCorrection = newValue; }
	    );
	    
	    //Misc 
	    Foldout miscSettingsFoldout = containerInstance.Query<Foldout>("MiscSettingsFoldout").First();
	    m_syncMaterialListToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    Contents.SyncMaterialList,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.SyncMaterialList = newValue; }
	    );
	    m_progressiveDisplayToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    Contents.ProgressiveDisplay,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.ProgressiveDisplay = newValue; }
	    );
	    m_loggingToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    Contents.Logging,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.Logging = newValue; }
	    );
	    m_profilingToggle = AddField<Toggle,bool>(fieldTemplate, miscSettingsFoldout, 
		    Contents.Profiling,
		    (MeshSyncPlayerConfig config, bool newValue) => { config.Profiling = newValue; }
	    );
	    
	    //Animation Tweak
	    Foldout animationTweakSettingsFoldout = containerInstance.Query<Foldout>("AnimationTweakSettingsFoldout").First();
	    m_animationTweakTimeScaleField = AddField<FloatField, float>(fieldTemplate, animationTweakSettingsFoldout, 
		    Contents.TweakTimeScale,
		    (MeshSyncPlayerConfig config, float newValue) => {
			    config.GetAnimationTweakSettings().TimeScale = newValue;
		    }
	    );
	    m_animationTweakTimeOffsetField = AddField<FloatField, float>(fieldTemplate, animationTweakSettingsFoldout, 
		    Contents.TweakTimeOffset,
		    (MeshSyncPlayerConfig config, float newValue) => {
			    config.GetAnimationTweakSettings().TimeOffset = newValue;
		    }
	    );
	    m_animationTweakDropStepField = AddField<IntegerField, int>(fieldTemplate, animationTweakSettingsFoldout, 
		    Contents.TweakDropStep,
		    (MeshSyncPlayerConfig config, int newValue) => {
			    config.GetAnimationTweakSettings().DropStep = newValue;
		    }
	    );
	    m_animationTweakReductionThresholdField = AddField<FloatField, float>(fieldTemplate, animationTweakSettingsFoldout, 
		    Contents.TweakReductionThreshold,
		    (MeshSyncPlayerConfig config, float newValue) => {
			    config.GetAnimationTweakSettings().ReductionThreshold = newValue;
		    }
	    );
	    m_animationTweakEraseFlatCurvesToggle = AddField<Toggle, bool>(fieldTemplate, animationTweakSettingsFoldout, 
		    Contents.TweakEraseFlatCurves,
		    (MeshSyncPlayerConfig config, bool newValue) => {
			    config.GetAnimationTweakSettings().EraseFlatCurves = newValue;
		    }
	    );
	    	       
	    UpdatePlayerConfigUIElements(MeshSyncPlayerType.SERVER);
	    
        root.Add(containerInstance);
    }

//----------------------------------------------------------------------------------------------------------------------	

	//Support Toggle, FloatField, etc
	private F AddField<F,V>(VisualTreeAsset template, VisualElement parent, GUIContent content,
		Action<MeshSyncPlayerConfig,V> onValueChanged) where F: VisualElement,INotifyValueChanged<V>, new()  
	{

		TemplateContainer templateInstance = template.CloneTree();
		VisualElement fieldContainer = templateInstance.Query<VisualElement>("FieldContainer").First();
//		F field = templateInstance.Query<F>().First();
		Label label = templateInstance.Query<Label>().First();
		label.text = content.text;
		label.tooltip = content.tooltip;
		
		F field = new F();
		field.AddToClassList("general-settings-field");
		field.RegisterValueChangedCallback((ChangeEvent<V> changeEvent) => {

			MeshSyncPlayerConfig config = field.userData as MeshSyncPlayerConfig;
			if (null == config) {
				Debug.LogError("[MeshSync] Field doesn't have the correct user data");
				return;
			}
			
			onValueChanged(config, changeEvent.newValue);
			MeshSyncRuntimeSettings.GetOrCreateSettings().SaveSettings();
		});		
		
		fieldContainer.Add(field);
		parent.Add(templateInstance);
		m_playerConfigUIElements.Add(field);		
		return field;
	}
	
//----------------------------------------------------------------------------------------------------------------------	
	private PopupField<T> AddPopupField<T>(VisualTreeAsset template, VisualElement parent, GUIContent content,
		List<T> options, Action<MeshSyncPlayerConfig,int> onValueChanged) 
	{

		TemplateContainer templateInstance = template.CloneTree();
		VisualElement fieldContainer = templateInstance.Query<VisualElement>("FieldContainer").First();
		PopupField<T> popupField = new PopupField<T>(options,options[0]);
		popupField.AddToClassList("general-settings-field");
		
		Label label = templateInstance.Query<Label>().First();
		label.text    = content.text;
		label.tooltip = content.tooltip;
		popupField.RegisterValueChangedCallback( ( ChangeEvent<T> changeEvent)  => {
		
			MeshSyncPlayerConfig config = popupField.userData as MeshSyncPlayerConfig;
			if (null == config) {
				Debug.LogError("[MeshSync] Toggle doesn't have the correct user data");
				return;
			}
			
			onValueChanged(config, popupField.index);
			MeshSyncRuntimeSettings.GetOrCreateSettings().SaveSettings();
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
	}

//----------------------------------------------------------------------------------------------------------------------	

	private void UpdatePlayerConfigUIElements(MeshSyncPlayerType playerType) {
		MeshSyncRuntimeSettings runtimeSettings = MeshSyncRuntimeSettings.GetOrCreateSettings();
		MeshSyncPlayerConfig config = runtimeSettings.GetDefaultPlayerConfig(playerType);
	
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
	
	private IntegerField m_serverPortField;
	private Toggle m_allowPublicAccessToggle;
	
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
