using System;
using UnityEngine;

namespace Unity.MeshSync {
    
[Serializable]
internal class MeshSyncPlayerConfig { 
    
    public bool SyncVisibility = true;
    public bool SyncTransform = true;
    public bool SyncCameras = true;
    public bool SyncLights = true;
    public bool SyncMeshes = true;
    public bool UpdateMeshColliders = true;
    public bool SyncMaterials = true;
    public bool FindMaterialFromAssets = true;

    [SerializeField] private int m_classVersion = 1;
}
    
} //end namespace