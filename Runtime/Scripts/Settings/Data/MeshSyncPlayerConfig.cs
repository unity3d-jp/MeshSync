﻿using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class MeshSyncPlayerConfig : ISerializationCallbackReceiver {

    internal MeshSyncPlayerConfig() {
        m_animationTweakSettings = new AnimationTweakSettings();        
    }
        
//----------------------------------------------------------------------------------------------------------------------    

    internal MeshSyncPlayerConfig(MeshSyncPlayerConfig other) {
        //Sync Settings
        SyncVisibility         = other.SyncVisibility;
        SyncTransform          = other.SyncTransform;
        SyncCameras            = other.SyncCameras;
        SyncLights             = other.SyncLights;
        SyncMeshes             = other.SyncMeshes;
        UpdateMeshColliders    = other.UpdateMeshColliders;

        //Import Settings   
        ImporterSettings         = new ModelImporterSettings(other.ImporterSettings); 
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
        if (m_meshSyncPlayerConfigVersion == CUR_MESHSYNC_PLAYER_CONFIG_VERSION)
            return;

        if (m_meshSyncPlayerConfigVersion < (int) MeshSyncPlayerConfigVersion.MODEL_IMPORTER_0_10_X) {
#pragma warning disable 612
            ImporterSettings.CreateMaterials = SyncMaterials;
#pragma warning restore 612
        }

        m_meshSyncPlayerConfigVersion = CUR_MESHSYNC_PLAYER_CONFIG_VERSION;
    }
    

//----------------------------------------------------------------------------------------------------------------------    
    internal AnimationTweakSettings GetAnimationTweakSettings() { return m_animationTweakSettings;}
    
//----------------------------------------------------------------------------------------------------------------------    
    //Sync Settings
    public bool SyncVisibility         = true;
    public bool SyncTransform          = true;
    public bool SyncCameras            = true;
    public bool SyncLights             = true;
    public bool SyncMeshes             = true;
    public bool UpdateMeshColliders    = true;
    [Obsolete] public bool SyncMaterials          = true;

    [SerializeField] internal ModelImporterSettings ImporterSettings;

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
    
}
} //end namespace