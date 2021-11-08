using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;

namespace Unity.MeshSync {
[Serializable]
internal abstract class MeshSyncPlayerConfig : ISerializationCallbackReceiver {

    internal MeshSyncPlayerConfig() {
        m_animationTweakSettings = new AnimationTweakSettings();
        InitComponentSyncSettings();
    }

    internal void InitComponentSyncSettings() {
        m_componentSyncSettings = new List<ComponentSyncSettings>() {
            new ComponentSyncSettings(), //Camera
            new ComponentSyncSettings(), //Lights
        };        
    }
        
//----------------------------------------------------------------------------------------------------------------------    

    internal MeshSyncPlayerConfig(MeshSyncPlayerConfig other) {
        //Sync Settings
        SyncVisibility       = other.SyncVisibility;
        SyncTransform        = other.SyncTransform;


        m_componentSyncSettings = new List<ComponentSyncSettings>();  
        for (int i = 0; i < SYNC_COUNT; ++i) {
            m_componentSyncSettings.Add(other.m_componentSyncSettings[i]);                 
        }
        
        SyncMeshes           = other.SyncMeshes;
        UpdateMeshColliders  = other.UpdateMeshColliders;

        //Import Settings   
        m_importerSettings       = new ModelImporterSettings(other.m_importerSettings); 
        AnimationInterpolation   = other.AnimationInterpolation;
        KeyframeReduction        = other.KeyframeReduction;
        ReductionThreshold       = other.ReductionThreshold;
        ReductionEraseFlatCurves = other.ReductionEraseFlatCurves;
        ZUpCorrection            = other.ZUpCorrection;


        //Misc
        SyncMaterialList   = other.SyncMaterialList;
        ProgressiveDisplay = other.ProgressiveDisplay;
        Logging            = other.Logging;
        Profiling          = other.Profiling;
        
        m_animationTweakSettings = new AnimationTweakSettings(other.GetAnimationTweakSettings());
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    public void OnBeforeSerialize() {
        m_meshSyncPlayerConfigVersion = CUR_MESHSYNC_PLAYER_CONFIG_VERSION;
    }

    public void OnAfterDeserialize() {
        
        //Validate
        if (null == m_componentSyncSettings || m_componentSyncSettings.Count != SYNC_COUNT) {
            InitComponentSyncSettings();
        }
        
        if (m_meshSyncPlayerConfigVersion == CUR_MESHSYNC_PLAYER_CONFIG_VERSION)
            return;

        if (m_meshSyncPlayerConfigVersion < (int) MeshSyncPlayerConfigVersion.MODEL_IMPORTER_0_10_X) {
#pragma warning disable 612
            m_importerSettings.CreateMaterials = SyncMaterials;

            m_componentSyncSettings[SYNC_CAMERA].CanCreate = m_componentSyncSettings[SYNC_CAMERA].CanUpdate = SyncCameras;  
            m_componentSyncSettings[SYNC_LIGHTS].CanCreate = m_componentSyncSettings[SYNC_LIGHTS].CanUpdate = SyncLights;  
#pragma warning restore 612
        }

        m_meshSyncPlayerConfigVersion = CUR_MESHSYNC_PLAYER_CONFIG_VERSION;
    }
    
    
//----------------------------------------------------------------------------------------------------------------------    
    internal AnimationTweakSettings GetAnimationTweakSettings() { return m_animationTweakSettings;}

    internal ModelImporterSettings GetModelImporterSettings() => m_importerSettings;

    internal void SetComponentSyncSettings(int index, ComponentSyncSettings settings) {
        Assert.IsNotNull(settings);
        Assert.IsTrue(index >= 0 && index<SYNC_COUNT);
        m_componentSyncSettings[index] = settings;
    }
    
    internal ComponentSyncSettings GetComponentSyncSettings(int index) => m_componentSyncSettings[index];
    
    
//----------------------------------------------------------------------------------------------------------------------    
    //Sync Settings
    public bool SyncVisibility = true;
    public bool SyncTransform  = true; //for Update
    
    [Obsolete] private bool SyncCameras = true;
    [Obsolete] private bool SyncLights  = true;
    
    public bool SyncMeshes  = true;


    [SerializeField] private List<ComponentSyncSettings> m_componentSyncSettings = null;
    
    
    public bool UpdateMeshColliders    = true;

    [Obsolete] public bool SyncMaterials          = true;
    [SerializeField] private ModelImporterSettings m_importerSettings = new ModelImporterSettings();

    //Import Settings   
    public int   AnimationInterpolation   = (int) InterpolationMode.Smooth;
    public bool  KeyframeReduction        = true;
    public float ReductionThreshold       = 0.001f;
    public bool  ReductionEraseFlatCurves = false;
    public int   ZUpCorrection            = (int) ZUpCorrectionMode.FlipYZ;


    //Misc
    public bool SyncMaterialList   = true;
    public bool ProgressiveDisplay = true;
    public bool Logging            = false;
    public bool Profiling          = false;
    
//----------------------------------------------------------------------------------------------------------------------    
    
    [SerializeField] AnimationTweakSettings m_animationTweakSettings;
    
#pragma warning disable 414           
    [SerializeField] private int m_meshSyncPlayerConfigVersion = (int)MeshSyncPlayerConfigVersion.NO_VERSIONING;
#pragma warning restore 414

    private const int CUR_MESHSYNC_PLAYER_CONFIG_VERSION = (int)MeshSyncPlayerConfigVersion.MODEL_IMPORTER_0_10_X; 

    enum MeshSyncPlayerConfigVersion {
        NO_VERSIONING = 0,
        MODEL_IMPORTER_0_10_X, //With ModelImporterSettings for version 0.10.x-preview
    }
    
//----------------------------------------------------------------------------------------------------------------------

    
    internal const int SYNC_CAMERA = 0;
    internal const int SYNC_LIGHTS = 1;
    internal const int SYNC_COUNT  = 2;

}
} //end namespace