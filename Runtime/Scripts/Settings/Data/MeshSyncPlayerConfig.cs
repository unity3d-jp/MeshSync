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
        SyncMaterials          = other.SyncMaterials;
        FindMaterialFromAssets = other.FindMaterialFromAssets;

        //Import Settings   
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

//----------------------------------------------------------------------------------------------------------------------    
    //Sync Settings
    public bool SyncVisibility         = true;
    public bool SyncTransform          = true;
    public bool SyncCameras            = true;
    public bool SyncLights             = true;
    public bool SyncMeshes             = true;
    public bool UpdateMeshColliders    = true;
    public bool SyncMaterials          = true;
    public bool FindMaterialFromAssets = true;

    //Import Settings   
    public int   AnimationInterpolation   = (int) InterpolationMode.Smooth;
    public bool  KeyframeReduction        = true;
    public float ReductionThreshold       = 0.001f;
    public bool  ReductionEraseFlatCurves = false;
    public int   ZUpCorrection            = (int) ZUpCorrectionMode.FlipYZ;


    //Misc
    public bool SyncMaterialList   = true;
    public bool ProgressiveDisplay = true;
    public bool Logging            = true;
    public bool Profiling          = false;

//----------------------------------------------------------------------------------------------------------------------    
    
    [SerializeField] AnimationTweakSettings m_animationTweakSettings;
    [SerializeField] internal readonly int ClassVersion = 1;
}
} //end namespace