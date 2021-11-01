using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class MeshSyncPlayerConfig {

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
    internal AnimationTweakSettings GetAnimationTweakSettings() { return m_animationTweakSettings;}

    internal void SetModelImporterSettings(ModelImporterSettings importerSettings) {
        m_importerSettings = importerSettings;
    }

    internal ModelImporterSettings GetModelImporterSettings() => m_importerSettings;
    
//----------------------------------------------------------------------------------------------------------------------    
    //Sync Settings
    public bool SyncVisibility         = true;
    public bool SyncTransform          = true;
    public bool SyncCameras            = true;
    public bool SyncLights             = true;
    public bool SyncMeshes             = true;
    public bool UpdateMeshColliders    = true;
    [Obsolete] public bool SyncMaterials          = true;

    [SerializeField] private ModelImporterSettings m_importerSettings;

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
    [SerializeField] private readonly int m_meshSyncPlayerConfigVersion = 1;
#pragma warning restore 414    
    
}
} //end namespace