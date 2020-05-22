using System;
using UnityEngine;

namespace Unity.MeshSync {
    
[Serializable]
internal class MeshSyncPlayerConfig { 
    
    //Sync Settings
    public bool SyncVisibility = true;
    public bool SyncTransform = true;
    public bool SyncCameras = true;
    public bool SyncLights = true;
    public bool SyncMeshes = true;
    public bool UpdateMeshColliders = true;
    public bool SyncMaterials = true;
    public bool FindMaterialFromAssets = true;

    //Import Settings   
    public int     AnimationInterpolation = (int) InterpolationMode.Smooth;
    public bool    KeyframeReduction = true;
    public float   ReductionThreshold = 0.001f;
    public bool    ReductionEraseFlatCurves = false;
    public int     ZUpCorrection = (int) ZUpCorrectionMode.FlipYZ;
    
#pragma warning disable 414
    [SerializeField] private readonly int m_classVersion = 1;
#pragma warning restore 414
}
    
} //end namespace