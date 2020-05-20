using System;

namespace Unity.MeshSync {
    
[Serializable]
internal class AssetSyncSettings {

    public bool SyncVisibility = true;
    public bool SyncTransform = true;
    public bool SyncCameras = true;
    public bool SyncLights = true;
    public bool SyncMeshes = true;
    public bool UpdateMeshColliders = true;
    public bool SyncPoints = true;
    public bool SyncMaterials = true;
    public bool FindMaterialFromAssets = true;


    //TODO-sin: 2020-5-20 The defaults for SceneCachePlayer is different
    //player.updateMeshColliders = false;
    //player.findMaterialFromAssets = false;
    
}
    
} //end namespace