using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class ModelImporterSettings {

    internal ModelImporterSettings() { }
    
    internal ModelImporterSettings(ModelImporterSettings other) {
        this.CreateMaterials = other.CreateMaterials;
        this.MaterialSearchMode = other.MaterialSearchMode;
    }

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] internal bool CreateMaterials = true;
    [SerializeField] internal AssetSearchMode MaterialSearchMode = AssetSearchMode.LOCAL;

//----------------------------------------------------------------------------------------------------------------------
    
    //properties
    internal const string CREATE_MATERIALS_PROP     = "CreateMaterials";
    internal const string MATERIAL_SEARCH_MODE_PROP = "MaterialSearchMode";
    
    
}

} //end namespace