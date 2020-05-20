using System;
using UnityEngine;

namespace Unity.MeshSync {
    
[Serializable]
internal class MeshSyncPlayerConfig { //: ScriptableObject{

    // internal static MeshSyncPlayerConfig GetOrCreateSettings() {
    //     var settings = AssetDatabase.LoadAssetAtPath<MyCustomSettings>(k_MyCustomSettingsPath);
    //     if (settings == null)
    //     {
    //         settings = ScriptableObject.CreateInstance<MeshSyncPlayerConfig>();
    //         settings.m_Number = 42;
    //         settings.m_SomeString = "The answer to the universe";
    //         AssetDatabase.CreateAsset(settings, k_MyCustomSettingsPath);
    //         AssetDatabase.SaveAssets();
    //     }
    //     return settings;        
    // }
    //
//----------------------------------------------------------------------------------------------------------------------
    
    
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