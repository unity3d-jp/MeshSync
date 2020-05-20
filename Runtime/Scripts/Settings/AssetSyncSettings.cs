using System;

namespace Unity.MeshSync {
    
[Serializable]
internal class AssetSyncSettings {

    public bool SyncVisibility = true;
    public bool SyncTransform = true;
    public bool SyncCameras = true;
    public bool SyncLights = true;
    public bool SyncMeshes = true;
    public bool SyncPoints = true;
    public bool SyncMaterials = true;

    
}
    
} //end namespace