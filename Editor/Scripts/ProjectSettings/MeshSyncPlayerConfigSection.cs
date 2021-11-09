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

        public static readonly GUIContent Visibility      = EditorGUIUtility.TrTextContent("Visibility");
        public static readonly GUIContent UpdateTransform = EditorGUIUtility.TrTextContent("Update Transform");

        public static readonly GUIContent[] ComponentSyncs = new [] {
            EditorGUIUtility.TrTextContent("Camera"),
            EditorGUIUtility.TrTextContent("Lights"),
        };

        public static readonly GUIContent UsePhysicalCameraParams = EditorGUIUtility.TrTextContent("Use Physical Camera Params");
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
        
        bool isSceneCachePlayerConfig = (m_playerType == MeshSyncPlayerType.CACHE_PLAYER);
        MeshSyncPlayerConfig config   = null;
        if (isSceneCachePlayerConfig) {
            config = MeshSyncProjectSettings.GetOrCreateSettings().GetDefaultSceneCachePlayerConfig();
        } else {
            config = MeshSyncProjectSettings.GetOrCreateSettings().GetDefaultServerConfig();
        }
            
        
        TemplateContainer containerInstance = InstantiateContainer(m_playerType);
        parent.Add(containerInstance);
            
        //Add server port	            	          
        Foldout syncSettingsFoldout = containerInstance.Query<Foldout>("SyncSettingsFoldout").First();

        //Sync	           
        AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.UpdateTransform, config.SyncTransform,
            (bool newValue) => { config.SyncTransform = newValue; }
        );

        {
            int i = MeshSyncPlayerConfig.SYNC_CAMERA;
            ComponentSyncSettings componentSyncSettings = config.GetComponentSyncSettings(i);
            AddComponentSyncSettingFields(syncSettingsFoldout, Contents.ComponentSyncs[i], componentSyncSettings);
        }

        AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.UsePhysicalCameraParams, config.IsPhysicalCameraParamsUsed(),
            (bool newValue) => { config.UsePhysicalCameraParams(newValue); },
            "inner-field-container"
        );
        
        {
            int i = MeshSyncPlayerConfig.SYNC_LIGHTS;
            ComponentSyncSettings componentSyncSettings = config.GetComponentSyncSettings(i);
            AddComponentSyncSettingFields(syncSettingsFoldout, Contents.ComponentSyncs[i], componentSyncSettings);
        }
        
        
        AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.Meshes,config.SyncMeshes,
            (bool newValue) => { config.SyncMeshes = newValue; }
        );
        AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.UpdateMeshColliders, config.UpdateMeshColliders,
            (bool newValue) => { config.UpdateMeshColliders = newValue; },
            "inner-field-container"
        );

        AddPlayerConfigField<Toggle,bool>(syncSettingsFoldout, Contents.Visibility, config.SyncVisibility,
            (bool newValue) => { config.SyncVisibility = newValue; }
        );
        
        //import
        Foldout importSettingsFoldout = containerInstance.Query<Foldout>("ImportSettingsFoldout").First();
        ModelImporterSettings modelImporterSettings = config.GetModelImporterSettings();

        AddPlayerConfigField<Toggle,bool>(importSettingsFoldout, 
            Contents.CreateMaterials,modelImporterSettings.CreateMaterials,
            (bool newValue) => { modelImporterSettings.CreateMaterials = newValue; }
        );
        AddPlayerConfigPopupField(importSettingsFoldout, Contents.MaterialSearchMode, m_assetSearchModeEnums,
                m_assetSearchModeEnums[(int) modelImporterSettings.MaterialSearchMode],
            (int newValue) => { modelImporterSettings.MaterialSearchMode = (AssetSearchMode) newValue; },
            "inner-field-container"
        );
        
        AddPlayerConfigPopupField(importSettingsFoldout, Contents.AnimationInterpolation, m_animationInterpolationEnums,
            m_animationInterpolationEnums[config.AnimationInterpolation],
            (int newValue) => { config.AnimationInterpolation = newValue; }
        );
        AddPlayerConfigField<Toggle,bool>(importSettingsFoldout, Contents.KeyframeReduction,config.KeyframeReduction,
            (bool newValue) => { config.KeyframeReduction = newValue; }
        );
        AddPlayerConfigField<FloatField, float>(importSettingsFoldout,
            Contents.ReductionThreshold,config.ReductionThreshold,
            (float newValue) => { config.ReductionThreshold = newValue; }
        );
        AddPlayerConfigField<Toggle,bool>(importSettingsFoldout, 
            Contents.ReductionEraseFlatCurves,config.ReductionEraseFlatCurves,
            (bool newValue) => { config.ReductionEraseFlatCurves = newValue; }
        );
        AddPlayerConfigPopupField(importSettingsFoldout, 
            Contents.ZUpCorrection, m_zUpCorrectionEnums,m_zUpCorrectionEnums[config.ZUpCorrection],
            (int newValue) => { config.ZUpCorrection = newValue; }
        );
        
        //Misc 
        Foldout miscSettingsFoldout = containerInstance.Query<Foldout>("MiscSettingsFoldout").First();
        AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, 
            Contents.SyncMaterialList,config.SyncMaterialList,
            (bool newValue) => { config.SyncMaterialList = newValue; }
        );
        AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, Contents.ProgressiveDisplay,config.ProgressiveDisplay,
            (bool newValue) => { config.ProgressiveDisplay = newValue; }
        );
        AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, Contents.Logging,config.Logging,
            (bool newValue) => { config.Logging = newValue; }
        );
        AddPlayerConfigField<Toggle,bool>(miscSettingsFoldout, Contents.Profiling,config.Profiling,
            (bool newValue) => { config.Profiling = newValue; }
        );
        
        //Animation Tweak
        Foldout atsFoldout = containerInstance.Query<Foldout>("AnimationTweakSettingsFoldout").First();
        AnimationTweakSettings ats = config.GetAnimationTweakSettings();
        AddPlayerConfigField<FloatField, float>(atsFoldout, Contents.TweakTimeScale,ats.TimeScale,
            (float newValue) => { ats.TimeScale = newValue; }
        );
        AddPlayerConfigField<FloatField, float>(atsFoldout, Contents.TweakTimeOffset,ats.TimeOffset,
            (float newValue) => { ats.TimeOffset = newValue; }
        );
        AddPlayerConfigField<IntegerField, int>(atsFoldout, Contents.TweakDropStep,ats.DropStep,
            (int newValue) => { ats.DropStep = newValue; }
        );
        AddPlayerConfigField<FloatField, float>(atsFoldout, Contents.TweakReductionThreshold,ats.ReductionThreshold,
            (float newValue) => { ats.ReductionThreshold = newValue; }
        );
        AddPlayerConfigField<Toggle, bool>(atsFoldout, Contents.TweakEraseFlatCurves,ats.EraseFlatCurves,
            (bool newValue) => { ats.EraseFlatCurves = newValue; }
        );
                
        if (!isSceneCachePlayerConfig) 
            return;
        
        //Additional UI for SceneCache
        SceneCachePlayerConfig scPlayerConfig = config as SceneCachePlayerConfig;
        Assert.IsNotNull(scPlayerConfig);
        Foldout timelineSettingsFoldout = containerInstance.Query<Foldout>("TimelineSettingsFoldout").First();
        AddPlayerConfigPopupField(timelineSettingsFoldout, 
            Contents.TimelineSnapToFrame, m_snapToFrameEnums,m_snapToFrameEnums[scPlayerConfig.TimelineSnapToFrame],
            (int newValue) => { scPlayerConfig.TimelineSnapToFrame = newValue;}
        );

    }

    
//----------------------------------------------------------------------------------------------------------------------	

    //Support Toggle, FloatField, etc
    private F AddPlayerConfigField<F,V>(VisualElement parent, GUIContent content, V initialValue,
        Action<V> onValueChanged, string containerClass = null) where F: VisualElement,INotifyValueChanged<V>, new() 
    {
        F field = UIElementsEditorUtility.AddField<F, V>(parent, content, initialValue, (ChangeEvent<V> changeEvent) => {

            F targetField = (changeEvent.target) as F;
            if (null == targetField)
                return;
                                        
            onValueChanged(changeEvent.newValue);
            MeshSyncProjectSettings.GetOrCreateSettings().Save();
        });

        field.AddToClassList("general-settings-field");
        if (!string.IsNullOrEmpty(containerClass)) {
            field.parent.AddToClassList(containerClass);
        }
        
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
        return popupField;
    }

    private static void AddComponentSyncSettingFields(VisualElement parent, GUIContent content, 
        ComponentSyncSettings componentSyncSettings) 
    {
        VisualTreeAsset template = UIElementsEditorUtility.LoadVisualTreeAsset(
            Constants.COMPONENT_SYNC_FIELDS_TEMPLATE_PATH);
        TemplateContainer templateInstance = template.CloneTree();                    
        VisualElement     fieldContainer   = templateInstance.Query<VisualElement>("FieldContainer").First();
        
        
        Label label = templateInstance.Query<Label>().First();
        label.text    = content.text;
        label.tooltip = content.tooltip;

        Toggle createToggle = templateInstance.Query<Toggle>("CreateToggle").First();
        Assert.IsNotNull(createToggle);

        createToggle.SetValueWithoutNotify(componentSyncSettings.CanCreate);
        createToggle.RegisterValueChangedCallback((ChangeEvent<bool> changeEvent) => {
            componentSyncSettings.CanCreate = changeEvent.newValue;
        });        

        Toggle updateToggle = templateInstance.Query<Toggle>("UpdateToggle").First();
        Assert.IsNotNull(updateToggle);
        updateToggle.SetValueWithoutNotify(componentSyncSettings.CanUpdate);
        updateToggle.RegisterValueChangedCallback((ChangeEvent<bool> changeEvent) => {
            componentSyncSettings.CanUpdate = changeEvent.newValue;
        });
        
        parent.Add(fieldContainer);
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
    
    
    private readonly MeshSyncPlayerType m_playerType;

    private readonly List<string> m_animationInterpolationEnums = new List<string>(Enum.GetNames( typeof( InterpolationMode )));
    private readonly List<string> m_zUpCorrectionEnums = new List<string>(Enum.GetNames( typeof( ZUpCorrectionMode )));
    private readonly List<string> m_assetSearchModeEnums = EnumUtility.ToInspectorNames(typeof(AssetSearchMode));

    private readonly List<string> m_snapToFrameEnums = EnumUtility.ToInspectorNames(typeof(SnapToFrame));
    
}

} //end namespace 
