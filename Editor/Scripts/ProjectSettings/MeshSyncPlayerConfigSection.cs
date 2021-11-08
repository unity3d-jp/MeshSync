using System;
using System.Collections.Generic;
using NUnit.Framework;
using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine;
using UnityEngine.UIElements;
using UnityEditor;
using UnityEditor.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;

namespace Unity.MeshSync.Editor {

internal class MeshSyncPlayerConfigSection {

    internal class Contents {

        public static readonly GUIContent Visibility          = EditorGUIUtility.TrTextContent("Visibility");
        public static readonly GUIContent Transform           = EditorGUIUtility.TrTextContent("Transform");

        public static readonly GUIContent[] ComponentSyncCreate = new [] {
            EditorGUIUtility.TrTextContent("Camera Create"),
            EditorGUIUtility.TrTextContent("Lights Create"),
        };

        public static readonly GUIContent[] ComponentSyncUpdate = new [] {
            EditorGUIUtility.TrTextContent("Camera Update"),
            EditorGUIUtility.TrTextContent("Lights Update"),
        };
        
        
        public static readonly GUIContent Meshes              = EditorGUIUtility.TrTextContent("Meshes");
        public static readonly GUIContent UpdateMeshColliders = EditorGUIUtility.TrTextContent("Update mesh colliders");

        //Import
        public static readonly GUIContent CreateMaterials    = EditorGUIUtility.TrTextContent("Create Materials");
        public static readonly GUIContent MaterialSearchMode = EditorGUIUtility.TrTextContent("Material Search Mode");
        
        public static readonly GUIContent AnimationInterpolation = EditorGUIUtility.TrTextContent("Animation interpolation");
        public static readonly GUIContent KeyframeReduction  = EditorGUIUtility.TrTextContent("Keyframe reduction");
        public static readonly GUIContent ReductionThreshold = EditorGUIUtility.TrTextContent("Reduction threshold");
        public static readonly GUIContent ReductionEraseFlatCurves = EditorGUIUtility.TrTextContent("Reduction erase flat curves");
        public static readonly GUIContent ZUpCorrection = EditorGUIUtility.TrTextContent("Z-Up correction");

        
        public static readonly GUIContent SyncMaterialList = EditorGUIUtility.TrTextContent("Sync material list");
        public static readonly GUIContent ProgressiveDisplay = EditorGUIUtility.TrTextContent("Progressive display");
        public static readonly GUIContent Logging = EditorGUIUtility.TrTextContent("Logging");
        public static readonly GUIContent Profiling = EditorGUIUtility.TrTextContent("Profiling");

        public static readonly GUIContent TweakTimeScale = EditorGUIUtility.TrTextContent("Time scale");
        public static readonly GUIContent TweakTimeOffset = EditorGUIUtility.TrTextContent("Time offset");
        public static readonly GUIContent TweakDropStep = EditorGUIUtility.TrTextContent("Drop step");
        public static readonly GUIContent TweakReductionThreshold = EditorGUIUtility.TrTextContent("Reduction threshold");
        public static readonly GUIContent TweakEraseFlatCurves = EditorGUIUtility.TrTextContent("Erase flat curves");

        public static readonly GUIContent TimelineSnapToFrame = EditorGUIUtility.TrTextContent("Snap To Frame");
        
    }

//----------------------------------------------------------------------------------------------------------------------
    
    internal MeshSyncPlayerConfigSection(MeshSyncPlayerType playerType) {
        m_playerType = playerType;
        
        
    }
    
//----------------------------------------------------------------------------------------------------------------------        
    internal void Setup(VisualElement parent) {
        m_playerConfigUIElements = new List<VisualElement>();
        
        bool isSceneCachePlayerConfig = (m_playerType == MeshSyncPlayerType.CACHE_PLAYER);
        MeshSyncPlayerConfig config = isSceneCachePlayerConfig ? 
            MeshSyncProjectSettings.GetOrCreateSettings().GetDefaultSceneCachePlayerConfig() : 
            MeshSyncProjectSettings.GetOrCreateSettings().GetDefaultServerConfig();
        
        TemplateContainer containerInstance = InstantiateContainer(m_playerType);
            
        //Add server port	            	          
        Foldout syncSettingsFoldout = containerInstance.Query<Foldout>("SyncSettingsFoldout").First();

        //Sync	           
        m_syncTransformToggle = AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.Transform, false,
            (bool newValue) => { config.SyncTransform = newValue; }
        );

        m_componentSyncSettingsUIList.Clear();
        for (int i = 0; i < MeshSyncPlayerConfig.SYNC_COUNT; ++i) {
            ComponentSyncSettingsUI syncSettings = new ComponentSyncSettingsUI(i);
            m_componentSyncSettingsUIList.Add(syncSettings);

            syncSettings.CanCreateToggle = AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, 
                Contents.ComponentSyncCreate[i],false,
                (bool newValue) => { config.GetComponentSyncSettings(syncSettings.SyncIndex).CanCreate = newValue; }
            );
            syncSettings.CanUpdateToggle = AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, 
                Contents.ComponentSyncUpdate[i],false,
                (bool newValue) => { config.GetComponentSyncSettings(syncSettings.SyncIndex).CanUpdate = newValue; }
            );
            
        }
        
        m_syncMeshesToggle = AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.Meshes,false,
            (bool newValue) => { config.SyncMeshes = newValue; }
        );
        m_updateMeshCollidersToggle = AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.UpdateMeshColliders, false,
            (bool newValue) => { config.UpdateMeshColliders = newValue; }
        );

        m_syncVisibilityToggle = AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.Visibility, false,
            (bool newValue) => { config.SyncVisibility = newValue; }
        );
        
        //import
        Foldout importSettingsFoldout = containerInstance.Query<Foldout>("ImportSettingsFoldout").First();

        m_createMaterialsToggle = AddPlayerConfigField<Toggle,bool>(importSettingsFoldout, 
            Contents.CreateMaterials,false,
            (bool newValue) => { config.GetModelImporterSettings().CreateMaterials = newValue; }
        );
        m_materialSearchModePopup = AddPlayerConfigPopupField(importSettingsFoldout, 
            Contents.MaterialSearchMode, m_assetSearchModeEnums,m_assetSearchModeEnums[0],
            (int newValue) => {
                config.GetModelImporterSettings().MaterialSearchMode = (AssetSearchMode) newValue;
            },
            "inner-field-container"
        );
        
        m_animationInterpolationPopup = AddPlayerConfigPopupField(importSettingsFoldout, 
            Contents.AnimationInterpolation, m_animationInterpolationEnums,m_animationInterpolationEnums[0],
            (int newValue) => { config.AnimationInterpolation = newValue; }
        );
        m_keyframeReductionToggle = AddPlayerConfigField<Toggle,bool>(importSettingsFoldout, 
            Contents.KeyframeReduction,false,
            (bool newValue) => { config.KeyframeReduction = newValue; }
        );
        m_reductionThresholdField = AddPlayerConfigField<FloatField, float>(importSettingsFoldout, 
            Contents.ReductionThreshold,0.0f,
            (float newValue) => { config.ReductionThreshold = newValue; }
        );
        m_reductionEraseFlatCurves = AddPlayerConfigField<Toggle,bool>(importSettingsFoldout, 
            Contents.ReductionEraseFlatCurves,false,
            (bool newValue) => { config.ReductionEraseFlatCurves = newValue; }
        );
        m_zUpCorrectionPopup = AddPlayerConfigPopupField(importSettingsFoldout, 
            Contents.ZUpCorrection, m_zUpCorrectionEnums,m_zUpCorrectionEnums[0],
            (int newValue) => { config.ZUpCorrection = newValue; }
        );
        
        //Misc 
        Foldout miscSettingsFoldout = containerInstance.Query<Foldout>("MiscSettingsFoldout").First();
        m_syncMaterialListToggle = AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, 
            Contents.SyncMaterialList,false,
            (bool newValue) => { config.SyncMaterialList = newValue; }
        );
        m_progressiveDisplayToggle = AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, 
            Contents.ProgressiveDisplay,false,
            (bool newValue) => { config.ProgressiveDisplay = newValue; }
        );
        m_loggingToggle = AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, Contents.Logging,false,
            (bool newValue) => { config.Logging = newValue; }
        );
        m_profilingToggle = AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, 
            Contents.Profiling,false,
            (bool newValue) => { config.Profiling = newValue; }
        );
        
        //Animation Tweak
        Foldout animationTweakSettingsFoldout = containerInstance.Query<Foldout>("AnimationTweakSettingsFoldout").First();
        AnimationTweakSettings animationTweakSettings = config.GetAnimationTweakSettings();
        m_animationTweakTimeScaleField = AddPlayerConfigField<FloatField, float>(animationTweakSettingsFoldout, 
            Contents.TweakTimeScale,animationTweakSettings.TimeScale,
            (float newValue) => { animationTweakSettings.TimeScale = newValue; }
        );
        m_animationTweakTimeOffsetField = AddPlayerConfigField<FloatField, float>(animationTweakSettingsFoldout, 
            Contents.TweakTimeOffset,animationTweakSettings.TimeOffset,
            (float newValue) => { animationTweakSettings.TimeOffset = newValue; }
        );
        m_animationTweakDropStepField = AddPlayerConfigField<IntegerField, int>(animationTweakSettingsFoldout, 
            Contents.TweakDropStep,animationTweakSettings.DropStep,
            (int newValue) => { animationTweakSettings.DropStep = newValue; }
        );
        m_animationTweakReductionThresholdField = AddPlayerConfigField<FloatField, float>(animationTweakSettingsFoldout, 
            Contents.TweakReductionThreshold,animationTweakSettings.ReductionThreshold,
            (float newValue) => { animationTweakSettings.ReductionThreshold = newValue; }
        );
        m_animationTweakEraseFlatCurvesToggle = AddPlayerConfigField<Toggle, bool>(animationTweakSettingsFoldout, 
            Contents.TweakEraseFlatCurves,animationTweakSettings.EraseFlatCurves,
            (bool newValue) => { animationTweakSettings.EraseFlatCurves = newValue; }
        );
                
        //Update the values in each UI elements
        if (isSceneCachePlayerConfig) {
            SceneCachePlayerConfig scPlayerConfig = config as SceneCachePlayerConfig;
            Assert.IsNotNull(scPlayerConfig);
            Foldout timelineSettingsFoldout = containerInstance.Query<Foldout>("TimelineSettingsFoldout").First();	    
            m_timelineSnapToFramePopup = AddPlayerConfigPopupField(timelineSettingsFoldout, 
                Contents.TimelineSnapToFrame, m_snapToFrameEnums,m_snapToFrameEnums[0],
                (int newValue) => { scPlayerConfig.TimelineSnapToFrame = newValue;}
            );
            
            UpdateSceneCacheUIElements();
        } else {
            UpdateServerUIElements();
        }
        
        parent.Add(containerInstance);
    }

    
//----------------------------------------------------------------------------------------------------------------------	

    //Support Toggle, FloatField, etc
    private F AddPlayerConfigField<F,V>(VisualElement parent, GUIContent content, V initialValue,
        Action<V> onValueChanged) where F: VisualElement,INotifyValueChanged<V>, new() 
    {
        F field = UIElementsEditorUtility.AddField<F, V>(parent, content, initialValue, (ChangeEvent<V> changeEvent) => {

            F targetField = (changeEvent.target) as F;
            if (null == targetField)
                return;
                            
            MeshSyncPlayerConfig config = targetField.userData as MeshSyncPlayerConfig;
            if (null == config) {
                Debug.LogError("[MeshSync] Field doesn't have the correct user data");
                return;
            }
            
            onValueChanged(changeEvent.newValue);
            MeshSyncProjectSettings.GetOrCreateSettings().Save();
        });

        field.AddToClassList("general-settings-field");
        m_playerConfigUIElements.Add(field);
        return field;
    }
    
//----------------------------------------------------------------------------------------------------------------------	
    private PopupField<T> AddPlayerConfigPopupField<T>(VisualElement parent, GUIContent content,
        List<T> options, T initialValue, Action<int> onValueChanged, 
        string containerClass = null)  
    {

        PopupField<T> popupField = UIElementsEditorUtility.AddPopupField<T>(parent, content, options, initialValue,
            (ChangeEvent<T> changeEvent) => {
                PopupField<T> targetField = (changeEvent.target) as PopupField<T>;
                if (null == targetField)
                    return;
            
                onValueChanged(targetField.index);
                MeshSyncProjectSettings.GetOrCreateSettings().Save();                
            }
        );        
        popupField.AddToClassList("general-settings-field");
        if (!string.IsNullOrEmpty(containerClass)) {
            popupField.parent.AddToClassList(containerClass);
        }
        m_playerConfigUIElements.Add(popupField);
        return popupField;
    }
    

//----------------------------------------------------------------------------------------------------------------------

    private void UpdateServerUIElements() {
        MeshSyncPlayerConfig config = MeshSyncProjectSettings.GetOrCreateSettings().GetDefaultServerConfig();	
        UpdateCommonUIElements(config);		
        SetupUIElementUserData(config);		
    }
    
    private void UpdateSceneCacheUIElements() {
        SceneCachePlayerConfig config = MeshSyncProjectSettings.GetOrCreateSettings().GetDefaultSceneCachePlayerConfig();	
        UpdateCommonUIElements(config);		
        m_timelineSnapToFramePopup.SetValueWithoutNotify(m_snapToFrameEnums[config.TimelineSnapToFrame]);
        SetupUIElementUserData(config);
    }

    private void UpdateCommonUIElements(MeshSyncPlayerConfig config) {
        //sync
        m_syncVisibilityToggle.SetValueWithoutNotify(config.SyncVisibility);
        m_syncTransformToggle.SetValueWithoutNotify(config.SyncTransform);

        foreach (ComponentSyncSettingsUI syncSettingsUI in m_componentSyncSettingsUIList) {
            int index = syncSettingsUI.SyncIndex;
            syncSettingsUI.CanCreateToggle.SetValueWithoutNotify(config.GetComponentSyncSettings(index).CanCreate);
            syncSettingsUI.CanUpdateToggle.SetValueWithoutNotify(config.GetComponentSyncSettings(index).CanUpdate);
        }
        
        m_syncMeshesToggle.SetValueWithoutNotify(config.SyncMeshes);
        m_updateMeshCollidersToggle.SetValueWithoutNotify(config.UpdateMeshColliders);

        //Import
        ModelImporterSettings importerSettings = config.GetModelImporterSettings();
        m_createMaterialsToggle.SetValueWithoutNotify(importerSettings.CreateMaterials);
        m_materialSearchModePopup.SetValueWithoutNotify(m_assetSearchModeEnums[(int)importerSettings.MaterialSearchMode]);
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
        
    }

    private void SetupUIElementUserData(MeshSyncPlayerConfig config) {
        foreach (VisualElement uiElement in m_playerConfigUIElements) {
            uiElement.userData = config;
        }		
    }
    
//----------------------------------------------------------------------------------------------------------------------

    private static TemplateContainer InstantiateContainer(MeshSyncPlayerType playerType) {

        VisualTreeAsset container = null;
        switch (playerType) {
            case MeshSyncPlayerType.SERVER: 
                container = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SERVER_CONFIG_CONTAINER_PATH);
                break; 
            case MeshSyncPlayerType.CACHE_PLAYER: 
                container = UIElementsEditorUtility.LoadVisualTreeAsset(Constants.SCENE_CACHE_PLAYER_CONFIG_CONTAINER_PATH);
                break;
            default : 
                Assert.Fail();
                break;
        }
        
        return container.CloneTree();		
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    //Sync Settings
    private Toggle m_syncVisibilityToggle;
    private Toggle m_syncTransformToggle;

    private readonly List<ComponentSyncSettingsUI> m_componentSyncSettingsUIList = new List<ComponentSyncSettingsUI>();  
        
    private Toggle m_syncMeshesToggle;
    private Toggle m_updateMeshCollidersToggle;
    
    //Import Settings
    private Toggle             m_createMaterialsToggle;
    private PopupField<string> m_materialSearchModePopup;
    
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
    private FloatField   m_animationTweakTimeScaleField;
    private FloatField   m_animationTweakTimeOffsetField;
    private IntegerField m_animationTweakDropStepField;
    private FloatField   m_animationTweakReductionThresholdField;
    private Toggle       m_animationTweakEraseFlatCurvesToggle;
    
    //Timeline
    private PopupField<string> m_timelineSnapToFramePopup;
    
    
    private readonly MeshSyncPlayerType m_playerType;
    private List<VisualElement> m_playerConfigUIElements;

    private readonly List<string> m_animationInterpolationEnums = new List<string>(Enum.GetNames( typeof( InterpolationMode )));
    private readonly List<string> m_zUpCorrectionEnums = new List<string>(Enum.GetNames( typeof( ZUpCorrectionMode )));
    private readonly List<string> m_assetSearchModeEnums = EnumUtility.ToInspectorNames(typeof(AssetSearchMode));

    private readonly List<string> m_snapToFrameEnums = EnumUtility.ToInspectorNames(typeof(SnapToFrame));
    
}

} //end namespace 
